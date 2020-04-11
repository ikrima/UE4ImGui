// Distributed under the MIT License (MIT) (see accompanying LICENSE file)


#include "Widgets/SImGuiHostWidget.h"
#include "UnrealImGui.h"
#include "SImGuiCanvasControl.h"

#include "ImGuiContextManager.h"
#include "ImGuiContextProxy.h"
#include "ImGuiInputHandler.h"
#include "ImGuiInteroperability.h"
#include "ImGuiModuleManager.h"
#include "ImGuiModuleSettings.h"
#include "TextureManager.h"
#include "Utilities/Arrays.h"
#include "Utilities/ScopeGuards.h"

#include <Engine/Console.h>

#include <utility>
#include "Framework/Application/SlateApplication.h"


// Hide ImGui Widget debug in non-developer mode.
#define IMGUI_WIDGET_DEBUG IMGUI_MODULE_DEVELOPER

#if IMGUI_WIDGET_DEBUG

DEFINE_LOG_CATEGORY_STATIC(LogImGuiWidget, Warning, All);

#define IMGUI_WIDGET_LOG(Verbosity, Format, ...) UE_LOG(LogImGuiWidget, Verbosity, Format, __VA_ARGS__)




namespace CVars
{
    TAutoConsoleVariable<int> DebugWidget(TEXT("ImGui.Debug.Widget"), 0,
        TEXT("Show debug for SImGuiWidget.\n")
        TEXT("0: disabled (default)\n")
        TEXT("1: enabled"),
        ECVF_Default);

    TAutoConsoleVariable<int> DebugInput(TEXT("ImGui.Debug.Input"), 0,
        TEXT("Show debug for input state.\n")
        TEXT("0: disabled (default)\n")
        TEXT("1: enabled"),
        ECVF_Default);
}

#else

#define IMGUI_WIDGET_LOG(...)

#endif // IMGUI_WIDGET_DEBUG


void SImGuiHostWidget::Construct(const FArguments& InArgs, TUniquePtr<FImGuiDrawer> InImGuiDrawer)
{
	checkf(InArgs._ModuleManager, TEXT("Null Module Manager argument"));

	ModuleManager = InArgs._ModuleManager;
  ContextProxy = MakeUnique<FImGuiContextProxy>(InArgs._ContextName, &ModuleManager->GetContextManager().GetFontAtlas(), MoveTemp(InImGuiDrawer));
	ContextProxy->SlateHostWidget = SharedThis(this);

	// Register for settings change.
	RegisterImGuiSettingsDelegates();

	// Get initial settings.
	const auto& Settings = ModuleManager->GetSettings();
	SetHideMouseCursor(Settings.UseSoftwareCursor());
    InputHandler = NewObject<UImGuiInputHandler>(GetTransientPackage());
    InputHandler->Initialize(ModuleManager, *(ContextProxy.Get()));
    InputHandler->AddToRoot();

	// Initialize state.
	UpdateVisibility();
	UpdateMouseCursor();

	ChildSlot
	[
		SAssignNew(CanvasControlWidget, SImGuiCanvasControl).OnTransformChanged(this, &SImGuiHostWidget::SetImGuiTransform)
	];

	ImGuiTransform = CanvasControlWidget->GetTransform();
}

SImGuiHostWidget::~SImGuiHostWidget()
{
	// Stop listening for settings change.
	UnregisterImGuiSettingsDelegates();

	// Release ImGui Input Handler.
    if (InputHandler.IsValid())
    {
        InputHandler->RemoveFromRoot();
        InputHandler.Reset();
    }
}

void SImGuiHostWidget::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
  Super::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

  // Manually update ImGui context to minimise lag between creating and rendering ImGui output. This will also
  // keep frame tearing at minimum because it is executed at the very end of the frame.
  ContextProxy->Tick(InDeltaTime, AllottedGeometry.GetAbsoluteSize());

  ImGuiRenderTransform = ImGuiTransform;
  UpdateMouseCursor();

  // HandleWindowFocusLost();
  // We can use window foreground status to notify about application losing or receiving focus. In some situations
  // we get mouse leave or enter events, but they are only sent if mouse pointer is inside of the viewport.
  if (bInputEnabled && HasKeyboardFocus()) {
    TSharedPtr<SWindow> ParentWindow = FSlateApplication::Get().FindWidgetWindow(AsShared());
    // if (bForegroundWindow != ParentWindow->IsActive())
    if (ParentWindow->GetNativeWindow()->IsForegroundWindow()) {
      InputHandler->OnKeyboardInputEnabled();
      InputHandler->OnGamepadInputEnabled();
    }
    else {
      InputHandler->OnKeyboardInputDisabled();
      InputHandler->OnGamepadInputDisabled();
    }
  }
  {
		FImGuiInputState& inputState = ContextProxy->GetInputState();
		inputState.ClearUpdateState();

    inputState.SetKeyboardNavigationEnabled(ModuleManager->GetProperties().IsKeyboardNavigationEnabled());
    inputState.SetGamepadNavigationEnabled(ModuleManager->GetProperties().IsGamepadNavigationEnabled());

    const auto& PlatformApplication = FSlateApplication::Get().GetPlatformApplication();
		inputState.SetGamepad(PlatformApplication.IsValid() && PlatformApplication->IsGamepadAttached());
  }
}

FReply SImGuiHostWidget::OnKeyChar(const FGeometry& MyGeometry, const FCharacterEvent& CharacterEvent)
{
	return InputHandler->OnKeyChar(CharacterEvent);
}

FReply SImGuiHostWidget::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& KeyEvent)
{
	UpdateCanvasControlMode(KeyEvent);
	return InputHandler->OnKeyDown(KeyEvent);
}

FReply SImGuiHostWidget::OnKeyUp(const FGeometry& MyGeometry, const FKeyEvent& KeyEvent)
{
	UpdateCanvasControlMode(KeyEvent);
	return InputHandler->OnKeyUp(KeyEvent);
}

FReply SImGuiHostWidget::OnAnalogValueChanged(const FGeometry& MyGeometry, const FAnalogInputEvent& AnalogInputEvent)
{
	return InputHandler->OnAnalogValueChanged(AnalogInputEvent);
}

FReply SImGuiHostWidget::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	return InputHandler->OnMouseButtonDown(MouseEvent).LockMouseToWidget(SharedThis(this));
}

FReply SImGuiHostWidget::OnMouseButtonDoubleClick(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	return InputHandler->OnMouseButtonDoubleClick(MouseEvent).LockMouseToWidget(SharedThis(this));
}

namespace
{
	bool NeedMouseLock(const FPointerEvent& MouseEvent)
	{
		return FSlateApplication::Get().GetPressedMouseButtons().Num() > 0;
	}
}

FReply SImGuiHostWidget::OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	FReply Reply = InputHandler->OnMouseButtonUp(MouseEvent);
	if (!NeedMouseLock(MouseEvent))
	{
		Reply.ReleaseMouseLock();
	}
	return Reply;
}

FReply SImGuiHostWidget::OnMouseWheel(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	return InputHandler->OnMouseWheel(MouseEvent);
}

FReply SImGuiHostWidget::OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	return InputHandler->OnMouseMove(TransformScreenPointToImGui(MyGeometry, MouseEvent.GetScreenSpacePosition()), MouseEvent);
}

FReply SImGuiHostWidget::OnFocusReceived(const FGeometry& MyGeometry, const FFocusEvent& FocusEvent)
{
	Super::OnFocusReceived(MyGeometry, FocusEvent);

	IMGUI_WIDGET_LOG(VeryVerbose, TEXT("ImGui Widget %s - Focus Received."), *ContextProxy->GetName());

	InputHandler->OnKeyboardInputEnabled();
	InputHandler->OnGamepadInputEnabled();

	FSlateApplication::Get().ResetToDefaultPointerInputSettings();
	return FReply::Handled();
}

void SImGuiHostWidget::OnFocusLost(const FFocusEvent& FocusEvent)
{
	Super::OnFocusLost(FocusEvent);

	IMGUI_WIDGET_LOG(VeryVerbose, TEXT("ImGui Widget %s - Focus Lost."), *ContextProxy->GetName());

	InputHandler->OnKeyboardInputDisabled();
	InputHandler->OnGamepadInputDisabled();
}

void SImGuiHostWidget::OnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	Super::OnMouseEnter(MyGeometry, MouseEvent);

	IMGUI_WIDGET_LOG(VeryVerbose, TEXT("ImGui Widget %s - Mouse Enter."), *ContextProxy->GetName());

	InputHandler->OnMouseInputEnabled();
}

void SImGuiHostWidget::OnMouseLeave(const FPointerEvent& MouseEvent)
{
	Super::OnMouseLeave(MouseEvent);

	IMGUI_WIDGET_LOG(VeryVerbose, TEXT("ImGui Widget %s - Mouse Leave."), *ContextProxy->GetName());

	InputHandler->OnMouseInputDisabled();
}

FReply SImGuiHostWidget::OnTouchStarted(const FGeometry& MyGeometry, const FPointerEvent& TouchEvent)
{
	return InputHandler->OnTouchStarted(TransformScreenPointToImGui(MyGeometry, TouchEvent.GetScreenSpacePosition()), TouchEvent);
}

FReply SImGuiHostWidget::OnTouchMoved(const FGeometry& MyGeometry, const FPointerEvent& TouchEvent)
{
	return InputHandler->OnTouchMoved(TransformScreenPointToImGui(MyGeometry, TouchEvent.GetScreenSpacePosition()), TouchEvent);
}

FReply SImGuiHostWidget::OnTouchEnded(const FGeometry& MyGeometry, const FPointerEvent& TouchEvent)
{
	UpdateVisibility();
	return InputHandler->OnTouchEnded(TransformScreenPointToImGui(MyGeometry, TouchEvent.GetScreenSpacePosition()), TouchEvent);
}

void SImGuiHostWidget::RegisterImGuiSettingsDelegates()
{
	auto& Settings = ModuleManager->GetSettings();

	if (!Settings.OnUseSoftwareCursorChanged.IsBoundToObject(this))
	{
		Settings.OnUseSoftwareCursorChanged.AddRaw(this, &SImGuiHostWidget::SetHideMouseCursor);
	}
}

void SImGuiHostWidget::UnregisterImGuiSettingsDelegates()
{
	auto& Settings = ModuleManager->GetSettings();

	Settings.OnUseSoftwareCursorChanged.RemoveAll(this);
}

void SImGuiHostWidget::SetHideMouseCursor(bool bHide)
{
	if (bHideMouseCursor != bHide)
	{
		bHideMouseCursor = bHide;
		UpdateMouseCursor();
	}
}

void SImGuiHostWidget::UpdateVisibility()
{
	// Make sure that we do not occlude other widgets, if input is disabled or if mouse is set to work in a transparent
	// mode (hit-test invisible).
	SetVisibility(bInputEnabled ? EVisibility::Visible : EVisibility::HitTestInvisible);

	IMGUI_WIDGET_LOG(VeryVerbose, TEXT("ImGui Widget %s - Visibility updated to '%s'."),
        *ContextProxy->GetName(), *GetVisibility().ToString());
}

void SImGuiHostWidget::UpdateMouseCursor()
{
	if (!bHideMouseCursor)
	{
		SetCursor(ContextProxy.IsValid() ? ContextProxy->GetMouseCursor() : EMouseCursor::Default);
	}
	else
	{
		SetCursor(EMouseCursor::None);
	}
}


void SImGuiHostWidget::UpdateCanvasControlMode(const FInputEvent& InputEvent)
{
	CanvasControlWidget->SetActive(InputEvent.IsLeftAltDown() && InputEvent.IsLeftShiftDown());
}


FVector2D SImGuiHostWidget::TransformScreenPointToImGui(const FGeometry& MyGeometry, const FVector2D& Point) const
{
	const FSlateRenderTransform ImGuiToScreen = ImGuiTransform.Concatenate(MyGeometry.GetAccumulatedRenderTransform());
	return ImGuiToScreen.Inverse().TransformPoint(Point);
}

namespace
{
	FORCEINLINE FSlateRenderTransform RoundTranslation(const FSlateRenderTransform& Transform)
	{
		const FVector2D& Translation = Transform.GetTranslation();
		return FSlateRenderTransform{ Transform.GetMatrix(),
			FVector2D{ FMath::RoundToFloat(Translation.X), FMath::RoundToFloat(Translation.Y) } };
	}
}

int32 SImGuiHostWidget::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyClippingRect,
	FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& WidgetStyle, bool bParentEnabled) const
{
	// Calculate transform from ImGui to screen space. Rounding translation is necessary to keep it pixel-perfect
	// in older engine versions.
	const FSlateRenderTransform& WidgetToScreen = AllottedGeometry.GetAccumulatedRenderTransform();
	const FSlateRenderTransform ImGuiToScreen = RoundTranslation(ImGuiRenderTransform.Concatenate(WidgetToScreen));


	for (const auto& DrawList : ContextProxy->GetDrawData())
	{
		DrawList.CopyVertexData(VertexBuffer, ImGuiToScreen);

		int IndexBufferOffset = 0;
		for (int CommandNb = 0; CommandNb < DrawList.NumCommands(); CommandNb++)
		{
			const auto& DrawCommand = DrawList.GetCommand(CommandNb, ImGuiToScreen);

			DrawList.CopyIndexData(IndexBuffer, IndexBufferOffset, DrawCommand.NumElements);

			// Advance offset by number of copied elements to position it for the next command.
			IndexBufferOffset += DrawCommand.NumElements;

			// Get texture resource handle for this draw command (null index will be also mapped to a valid texture).
			const FSlateResourceHandle& Handle = ModuleManager->GetTextureManager().GetTextureHandle(DrawCommand.TextureId);

			// Transform clipping rectangle to screen space and apply to elements that we draw.
			const FSlateRect ClippingRect = DrawCommand.ClippingRect.IntersectionWith(MyClippingRect);

			OutDrawElements.PushClip(FSlateClippingZone{ ClippingRect });

			// Add elements to the list.
			FSlateDrawElement::MakeCustomVerts(OutDrawElements, LayerId, Handle, VertexBuffer, IndexBuffer, nullptr, 0, 0);

			OutDrawElements.PopClip();
		}
	}
	

	return Super::OnPaint(Args, AllottedGeometry, MyClippingRect, OutDrawElements, LayerId, WidgetStyle, bParentEnabled);
}


#undef IMGUI_WIDGET_LOG
