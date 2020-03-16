// Distributed under the MIT License (MIT) (see accompanying LICENSE file)


#include "ImGuiContextProxy.h"
#include "UnrealImGui.h"

#include "ImGuiInteroperability.h"
#include "Utilities/Arrays.h"

#include <Runtime/Launch/Resources/Version.h>
#include "Misc/Paths.h"
#include "GenericPlatform/GenericPlatformFile.h"


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

		FString Directory = FPaths::Combine(*SavedDir, TEXT("ImGui"));

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
        ImGuiIO& IO	= ImGui::GetIO();
	    IO.DisplaySize = ImVec2(DisplaySize.X, DisplaySize.Y);
	    IO.DeltaTime   = DeltaSeconds;

	    ImGuiInterops::CopyInput(IO, InputState);
	    InputState.ClearUpdateState();
	    ImGui::NewFrame();

	    ImGui::SetNextWindowPos(ImVec2(0, 0));
	    ImGui::SetNextWindowSize(IO.DisplaySize);
	    ImGui::Begin("Content", nullptr,
		    ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse
			    | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBringToFrontOnFocus);
    }

    // Tick Drawer 
	{
		DrawerObj->OnTick(DeltaSeconds);
		ThemeStyle.OnBegin();
		DrawerObj->OnDraw();
		ThemeStyle.OnEnd();
	}

	// Ending frame will produce render output that we capture and store for later use. This also puts context to
	// state in which it does not allow to draw controls, so we want to immediately start a new frame.
    {
	    ImGui::End();

	    // Prepare draw data (after this call we cannot draw to this context until we start a new frame).
	    ImGui::Render();

	    // Update our draw data, so we can use them later during Slate rendering while ImGui is in the middle of the
	    // next frame.
	    UpdateDrawData(ImGui::GetDrawData());

    #ifdef IMGUI_HAS_DOCK
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