// Distributed under the MIT License (MIT) (see accompanying LICENSE file)


#include "ImGuiContextProxy.h"
#include "UnrealImGui.h"

#include "ImGuiInteroperability.h"
#include "Utilities/Arrays.h"

#include <Runtime/Launch/Resources/Version.h>
#include "Misc/Paths.h"
#include "GenericPlatform/GenericPlatformFile.h"
#include "ImGuiModule.h"
#include "Widgets/SImGuiHostWidget.h"
#include "Framework/Application/SlateApplication.h"


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

FImGuiContextProxy::FImGuiContextProxy(const FString& InName, ImFontAtlas* InFontAtlas, TUniquePtr<FImGuiDrawer> InDrawer)
    : Name(InName), IniFilename(TCHAR_TO_ANSI(*GetIniFile(InName))), DrawerObj(MoveTemp(InDrawer))
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
        DrawerObj->imGuiCtxProxy = this;
        DrawerObj->OnInitialize();
    }
}

FImGuiContextProxy::~FImGuiContextProxy()
{
    if (Context)
    {
        // It seems that to properly shutdown context we need to set it as the current one (at least in this framework
        // version), even though we can pass it to the destroy function.
        SetAsCurrent();

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
        
        bWantsMouseCapture = IO.WantCaptureMouse;
    }
    
    // Draw Widget
    {
        DrawerObj->OnTick(DeltaSeconds);
        DrawerObj->OnDraw();
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


static TArray<FKey> GetImGuiMappedKeys()
{
	TArray<FKey> Keys;
	Keys.Reserve(Utilities::ArraySize<ImGuiInterops::ImGuiTypes::FKeyMap>::value + 8);

	// ImGui IO key map.
	Keys.Emplace(EKeys::Tab);
	Keys.Emplace(EKeys::Left);
	Keys.Emplace(EKeys::Right);
	Keys.Emplace(EKeys::Up);
	Keys.Emplace(EKeys::Down);
	Keys.Emplace(EKeys::PageUp);
	Keys.Emplace(EKeys::PageDown);
	Keys.Emplace(EKeys::Home);
	Keys.Emplace(EKeys::End);
	Keys.Emplace(EKeys::Delete);
	Keys.Emplace(EKeys::BackSpace);
	Keys.Emplace(EKeys::Enter);
	Keys.Emplace(EKeys::Escape);
	Keys.Emplace(EKeys::A);
	Keys.Emplace(EKeys::C);
	Keys.Emplace(EKeys::V);
	Keys.Emplace(EKeys::X);
	Keys.Emplace(EKeys::Y);
	Keys.Emplace(EKeys::Z);

	// Modifier keys.
	Keys.Emplace(EKeys::LeftShift);
	Keys.Emplace(EKeys::RightShift);
	Keys.Emplace(EKeys::LeftControl);
	Keys.Emplace(EKeys::RightControl);
	Keys.Emplace(EKeys::LeftAlt);
	Keys.Emplace(EKeys::RightAlt);
	Keys.Emplace(EKeys::LeftCommand);
	Keys.Emplace(EKeys::RightCommand);

	return Keys;
}

// Column layout utilities.
namespace Columns
{
	template<typename FunctorType>
	static void CollapsingGroup(const char* Name, int Columns, FunctorType&& DrawContent)
	{
		if (ImGui::CollapsingHeader(Name, ImGuiTreeNodeFlags_DefaultOpen))
		{
			const int LastColumns = ImGui::GetColumnsCount();
			ImGui::Columns(Columns, nullptr, false);
			DrawContent();
			ImGui::Columns(LastColumns);
		}
	}
}

// Controls tweaked for 2-columns layout.
namespace TwoColumns
{
	template<typename FunctorType>
	static inline void CollapsingGroup(const char* Name, FunctorType&& DrawContent)
	{
		Columns::CollapsingGroup(Name, 2, std::forward<FunctorType>(DrawContent));
	}

	namespace
	{
		void LabelText(const char* Label)
		{
			ImGui::Text("%s:", Label);
		}

		void LabelText(const wchar_t* Label)
		{
			ImGui::Text("%ls:", Label);
		}
	}

	template<typename LabelType>
	static void Value(LabelType&& Label, int32 Value)
	{
		LabelText(Label); ImGui::NextColumn();
		ImGui::Text("%d", Value); ImGui::NextColumn();
	}

	template<typename LabelType>
	static void Value(LabelType&& Label, uint32 Value)
	{
		LabelText(Label); ImGui::NextColumn();
		ImGui::Text("%u", Value); ImGui::NextColumn();
	}

	template<typename LabelType>
	static void Value(LabelType&& Label, float Value)
	{
		LabelText(Label); ImGui::NextColumn();
		ImGui::Text("%f", Value); ImGui::NextColumn();
	}

	template<typename LabelType>
	static void Value(LabelType&& Label, bool bValue)
	{
		LabelText(Label); ImGui::NextColumn();
		ImGui::Text("%ls", ((bValue) ? TEXT("true") : TEXT("false"))); ImGui::NextColumn();
	}

	template<typename LabelType>
	static void Value(LabelType&& Label, const TCHAR* Value)
	{
		LabelText(Label); ImGui::NextColumn();
		ImGui::Text("%ls", Value); ImGui::NextColumn();
	}
}

namespace Styles
{
	template<typename FunctorType>
	static void TextHighlight(bool bHighlight, FunctorType&& DrawContent)
	{
		if (bHighlight)
		{
			ImGui::PushStyleColor(ImGuiCol_Text, { 1.f, 1.f, 0.5f, 1.f });
		}
		DrawContent();
		if (bHighlight)
		{
			ImGui::PopStyleColor();
		}
	}
}

void FImGuiContextProxy::ShowImGuiDbgInputState() const
{
	const ImVec4 HiglightColor{ 1.f, 1.f, 0.5f, 1.f };
	Columns::CollapsingGroup("Mapped Keys", 4, [&]()
	{
		static const auto& Keys = GetImGuiMappedKeys();

		const int32 Num = Keys.Num();

		// Simplified when slicing for two 2.
		const int32 RowsNum = (Num + 1) / 2;

		for (int32 Row = 0; Row < RowsNum; Row++)
		{
			for (int32 Col = 0; Col < 2; Col++)
			{
				const int32 Idx = Row + Col * RowsNum;
				if (Idx < Num)
				{
					const FKey& Key = Keys[Idx];
					const uint32 KeyIndex = ImGuiInterops::GetKeyIndex(Key);
					Styles::TextHighlight(InputState.GetKeys()[KeyIndex], [&]()
					{
						TwoColumns::Value(*Key.GetDisplayName().ToString(), KeyIndex);
					});
				}
				else
				{
					ImGui::NextColumn(); ImGui::NextColumn();
				}
			}
		}
	});

	Columns::CollapsingGroup("Modifier Keys", 4, [&]()
	{
		Styles::TextHighlight(InputState.IsShiftDown(), [&]() { ImGui::Text("Shift"); }); ImGui::NextColumn();
		Styles::TextHighlight(InputState.IsControlDown(), [&]() { ImGui::Text("Control"); }); ImGui::NextColumn();
		Styles::TextHighlight(InputState.IsAltDown(), [&]() { ImGui::Text("Alt"); }); ImGui::NextColumn();
		ImGui::NextColumn();
	});

	Columns::CollapsingGroup("Mouse Buttons", 4, [&]()
	{
		static const FKey Buttons[] = { EKeys::LeftMouseButton, EKeys::RightMouseButton,
			EKeys::MiddleMouseButton, EKeys::ThumbMouseButton, EKeys::ThumbMouseButton2 };

		const int32 Num = Utilities::GetArraySize(Buttons);

		// Simplified when slicing for two 2.
		const int32 RowsNum = (Num + 1) / 2;

		for (int32 Row = 0; Row < RowsNum; Row++)
		{
			for (int32 Col = 0; Col < 2; Col++)
			{
				const int32 Idx = Row + Col * RowsNum;
				if (Idx < Num)
				{
					const FKey& Button = Buttons[Idx];
					const uint32 MouseIndex = ImGuiInterops::GetMouseIndex(Button);
					Styles::TextHighlight(InputState.GetMouseButtons()[MouseIndex], [&]()
					{
						TwoColumns::Value(*Button.GetDisplayName().ToString(), MouseIndex);
					});
				}
				else
				{
					ImGui::NextColumn(); ImGui::NextColumn();
				}
			}
		}
	});

	Columns::CollapsingGroup("Mouse Axes", 4, [&]()
	{
		TwoColumns::Value("Position X", InputState.GetMousePosition().X);
		TwoColumns::Value("Position Y", InputState.GetMousePosition().Y);
		TwoColumns::Value("Wheel Delta", InputState.GetMouseWheelDelta());
		ImGui::NextColumn(); ImGui::NextColumn();
	});

}


void FImGuiContextProxy::ShowSlateHostDbgWindow() const
{
  TSharedPtr<SImGuiHostWidget> slateHost = SlateHostWidget.Pin();
  ImGui::Spacing();

  TwoColumns::CollapsingGroup("Context", [&]() { TwoColumns::Value("Context Name", *this->GetName()); });

  TwoColumns::CollapsingGroup("Input Mode", [&]() { TwoColumns::Value("Input Enabled", slateHost->bInputEnabled); });

  TwoColumns::CollapsingGroup("Widget", [&]() {
    TwoColumns::Value("Visibility", *slateHost->GetVisibility().ToString());
    TwoColumns::Value("Is Hovered", slateHost->IsHovered());
    TwoColumns::Value("Is Directly Hovered", slateHost->IsDirectlyHovered());
    TwoColumns::Value("Has Keyboard Input", slateHost->HasKeyboardFocus());
  });

  TwoColumns::CollapsingGroup("Window", [&]() {
    const TSharedPtr<SWindow>& ParentWindow = FSlateApplication::Get().FindWidgetWindow(slateHost.ToSharedRef());
    TwoColumns::Value("Is Foreground Window", ParentWindow->GetNativeWindow()->IsForegroundWindow());
    TwoColumns::Value("Is Hovered", ParentWindow->IsHovered());
    TwoColumns::Value("Is Directly Hovered", ParentWindow->IsDirectlyHovered());
    TwoColumns::Value("Has Mouse Capture", ParentWindow->HasMouseCapture());
    TwoColumns::Value("Has Keyboard Input", ParentWindow->HasKeyboardFocus());
    TwoColumns::Value("Has Focused Descendants", ParentWindow->HasFocusedDescendants());
    auto Widget = slateHost->PreviousUserFocusedWidget.Pin();
    TwoColumns::Value("Previous User Focused", Widget.IsValid() ? *Widget->GetTypeAsString() : TEXT("None"));
  });
}
