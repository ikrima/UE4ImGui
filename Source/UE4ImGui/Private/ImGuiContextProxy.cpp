// Distributed under the MIT License (MIT) (see accompanying LICENSE file)


#include "ImGuiContextProxy.h"
#include "UnrealImGui.h"

#include "ImGuiInteroperability.h"
#include "Utilities/Arrays.h"

#include <Runtime/Launch/Resources/Version.h>
#include "Misc/Paths.h"
#include "GenericPlatform/GenericPlatformFile.h"
#include "ImGuiModule.h"


static constexpr float DEFAULT_CANVAS_WIDTH  = 3840.f;
static constexpr float DEFAULT_CANVAS_HEIGHT = 2160.f;


namespace
{
    FString GetSaveDirectory()
    {
#if ENGINE_COMPATIBILITY_LEGACY_SAVED_DIR
        const FString SavedDir = FPaths::GameSavedDir();
#else
        const FString SavedDir = FPaths::ProjectSavedDir();
#endif

        FString Directory = FPaths::Combine(*SavedDir, FImGuiModule::ModuleName);

        // Make sure that directory is created.
        IPlatformFile::GetPlatformPhysical().CreateDirectory(*Directory);

        return Directory;
    }

    FString GetIniFile(const FString& Name)
    {
        static FString SaveDirectory = GetSaveDirectory();
        return FPaths::Combine(SaveDirectory, Name + TEXT(".ini"));
    }
}	// namespace

FImGuiContextProxy::FImGuiContextProxy(const FString& InName, ImFontAtlas* InFontAtlas, TUniquePtr<FImGuiDrawer> InDrawer, const FImGuiThemeStyle& InThemeStyle)
    : Name(InName), IniFilename(TCHAR_TO_ANSI(*GetIniFile(InName))), DrawerObj(MoveTemp(InDrawer)), ThemeStyle(InThemeStyle)
{
    // Create context.
    Context = ImGui::CreateContext(InFontAtlas);

    // Set this context in ImGui for initialization (any allocations will be tracked in this context).
    SetAsCurrent();

    // Start initialization.
    ImGuiIO& IO = ImGui::GetIO();

    // Set session data storage.
    IO.IniFilename = IniFilename.c_str();

    // Use pre-defined canvas size.
    IO.DisplaySize = {DEFAULT_CANVAS_WIDTH, DEFAULT_CANVAS_HEIGHT};
    DisplaySize	= ImGuiInterops::ToVector2D(IO.DisplaySize);

    // Initialize key mapping, so context can correctly interpret input state.
    ImGuiInterops::SetUnrealKeyMap(IO);

    // Begin frame to complete context initialization (this is to avoid problems with other systems calling to ImGui
    // during startup).

    if (DrawerObj.IsValid())
    {
        DrawerObj->OnInitialize();
    }

    ThemeStyle.OnInit(*InFontAtlas);
}

FImGuiContextProxy::~FImGuiContextProxy()
{
    if (Context)
    {
        // It seems that to properly shutdown context we need to set it as the current one (at least in this framework
        // version), even though we can pass it to the destroy function.
        SetAsCurrent();

        ThemeStyle.OnDestroy();

        if (DrawerObj.IsValid())
        {
            DrawerObj->OnDestroy();
            DrawerObj.Reset();
        }


        // Save context data and destroy.
        ImGui::DestroyContext(Context);
    }
}

void FImGuiContextProxy::Tick(float DeltaSeconds, const FVector2D& InDisplaySize)
{
    SetAsCurrent();

    // Begin a new frame and set the context back to a state in which it allows to draw controls.
    {
        ImGuiIO& IO = ImGui::GetIO();
        IO.DisplaySize = ImVec2(DisplaySize.X, DisplaySize.Y);
        IO.DeltaTime = DeltaSeconds;

        ImGuiInterops::CopyInput(IO, InputState);
        InputState.ClearUpdateState();
        ImGui::NewFrame();
    }

    // Create MainWindowHost that everything will dock to
    {
        constexpr bool opt_fullscreen = true;

        // We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
        // because it would be confusing to have two docking targets within each others.
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
        if (opt_fullscreen)
        {
            ImGuiViewport* viewport = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos(viewport->Pos);
            ImGui::SetNextWindowSize(viewport->Size);
            ImGui::SetNextWindowViewport(viewport->ID);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
            window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
            window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
        }

        // When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background 
        // and handle the pass-thru hole, so we ask Begin() to not render a background.
        if (dockspaceFlags & ImGuiDockNodeFlags_PassthruCentralNode)
            window_flags |= ImGuiWindowFlags_NoBackground;

        // Important: note that we proceed even if Begin() returns false (aka window is collapsed).
        // This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
        // all active windows docked into it will lose their parent and become undocked.
        // We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
        // any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin(ImGuiUX::GetMainHostWindowName(), nullptr, window_flags);
        {
            ImGui::PopStyleVar();

            if (opt_fullscreen)
                ImGui::PopStyleVar(2);

            // DockSpace
            {
                ImGuiID dockspace_id = ImGui::GetID(ImGuiUX::GetMainHostDockSpaceName());
                ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspaceFlags);
            }
                
            if (ImGui::BeginMainMenuBar())
            {
                if (ImGui::BeginMenu("File"))
                {
                    if (ImGui::MenuItem("New")) {}
                    if (ImGui::MenuItem("Open", "Ctrl+O")) {}
                    if (ImGui::MenuItem("Save", "Ctrl+S")) {}
                    if (ImGui::MenuItem("Close")) {}
                    ImGui::EndMenu();
                }

                ImGui::EndMainMenuBar();
            }

            if (bShowAppMetrics) { ImGui::ShowMetricsWindow(&bShowAppMetrics); }
            if (bShowDemoWindow) { ImGui::ShowDemoWindow(&bShowDemoWindow); }
        }
        ImGui::End();
    }
    
    // Draw Widget
    {
        DrawerObj->OnTick(DeltaSeconds);
        ThemeStyle.OnBegin();
        DrawerObj->OnDraw();
        ThemeStyle.OnEnd();
    }

    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("Debug"))
        {
            ImGui::MenuItem("Metrics", NULL, &bShowAppMetrics);
            ImGui::MenuItem("DemoWindow", NULL, &bShowDemoWindow);
            ImGui::Separator();

            if (ImGui::MenuItem("Flag: NoSplit",                "", (dockspaceFlags & ImGuiDockNodeFlags_NoSplit)                != 0)) { dockspaceFlags ^= ImGuiDockNodeFlags_NoSplit;					}
            if (ImGui::MenuItem("Flag: NoResize",               "", (dockspaceFlags & ImGuiDockNodeFlags_NoResize)               != 0)) { dockspaceFlags ^= ImGuiDockNodeFlags_NoResize;				}
            if (ImGui::MenuItem("Flag: NoDockingInCentralNode", "", (dockspaceFlags & ImGuiDockNodeFlags_NoDockingInCentralNode) != 0)) { dockspaceFlags ^= ImGuiDockNodeFlags_NoDockingInCentralNode;	}
            if (ImGui::MenuItem("Flag: PassthruCentralNode",    "", (dockspaceFlags & ImGuiDockNodeFlags_PassthruCentralNode)    != 0)) { dockspaceFlags ^= ImGuiDockNodeFlags_PassthruCentralNode;		}
            if (ImGui::MenuItem("Flag: AutoHideTabBar",         "", (dockspaceFlags & ImGuiDockNodeFlags_AutoHideTabBar)         != 0)) { dockspaceFlags ^= ImGuiDockNodeFlags_AutoHideTabBar;          }
            ImGui::Separator();

            ImGui::EndMenu();
        }

        const float frameRate = ImGui::GetIO().Framerate;
        ImGui::Text("%02.2fms (%03.2fFPS)", frameRate, frameRate ? 1000.0f / frameRate : 0.0f);

        ImGui::EndMainMenuBar();
    }

    // Ending frame will produce render output that we capture and store for later use. This also puts context to
    // state in which it does not allow to draw controls, so we want to immediately start a new frame.
    {
        // Prepare draw data (after this call we cannot draw to this context until we start a new frame).
        ImGui::Render();

        // Update our draw data, so we can use them later during Slate rendering while ImGui is in the middle of the
        // next frame.
        UpdateDrawData(ImGui::GetDrawData());

    #if IMGUI_HAS_VIEWPORT
        // Update and Render additional Platform Windows
        if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
        }
    #endif
    }

    // Update context information (some data, like mouse cursor, may be cleaned in new frame, so we should collect it
    // beforehand).
    bHasActiveItem			  = ImGui::IsAnyItemActive();
    bIsMouseHoveringAnyWindow = ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow);
    MouseCursor				  = ImGuiInterops::ToSlateMouseCursor(ImGui::GetMouseCursor());
    DisplaySize				  = InDisplaySize;
}


void FImGuiContextProxy::UpdateDrawData(ImDrawData* DrawData)
{
    if (DrawData && DrawData->CmdListsCount > 0)
    {
        DrawLists.SetNum(DrawData->CmdListsCount, false);

        for (int Index = 0; Index < DrawData->CmdListsCount; Index++)
        {
            DrawLists[Index].TransferDrawData(*DrawData->CmdLists[Index]);
        }
    }
    else
    {
        // If we are not rendering then this might be a good moment to empty the array.
        DrawLists.Empty();
    }
}