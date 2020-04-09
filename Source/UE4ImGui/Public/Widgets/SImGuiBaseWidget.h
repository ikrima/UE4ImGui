// Distributed under the MIT License (MIT) (see accompanying LICENSE file)

#pragma once

#include "ImGuiModuleSettings.h"

#include <Widgets/SCompoundWidget.h>
#include "ImGuiContextProxy.h"
#include "Templates/UniquePtr.h"

// Hide ImGui Widget debug in non-developer mode.
#define IMGUI_WIDGET_DEBUG IMGUI_MODULE_DEVELOPER

class FImGuiModuleManager;
class SImGuiCanvasControl;
class UImGuiInputHandler;

// Slate widget for rendering ImGui output and storing Slate inputs.
class SImGuiBaseWidget : public SCompoundWidget
{
	typedef SCompoundWidget Super;

public:

	SLATE_BEGIN_ARGS(SImGuiBaseWidget)
	{}
	SLATE_ARGUMENT(FImGuiModuleManager*, ModuleManager)
    SLATE_ARGUMENT(FString, ContextName)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, TUniquePtr<FImGuiDrawer> InImGuiDrawer);

	~SImGuiBaseWidget();

	//----------------------------------------------------------------------------------------------------
	// SWidget overrides
	//----------------------------------------------------------------------------------------------------

	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

	virtual bool SupportsKeyboardFocus() const override { return bInputEnabled; }

	virtual FReply OnKeyChar(const FGeometry& MyGeometry, const FCharacterEvent& CharacterEvent) override;

	virtual FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& KeyEvent) override;

	virtual FReply OnKeyUp(const FGeometry& MyGeometry, const FKeyEvent& KeyEvent) override;

	virtual FReply OnAnalogValueChanged(const FGeometry& MyGeometry, const FAnalogInputEvent& AnalogInputEvent) override;

	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;

	virtual FReply OnMouseButtonDoubleClick(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;

	virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;

	virtual FReply OnMouseWheel(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;

	virtual FReply OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;

	virtual FReply OnFocusReceived(const FGeometry& MyGeometry, const FFocusEvent& FocusEvent) override;

	virtual void OnFocusLost(const FFocusEvent& FocusEvent) override;

	virtual void OnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;

	virtual void OnMouseLeave(const FPointerEvent& MouseEvent) override;

	virtual FReply OnTouchStarted(const FGeometry& MyGeometry, const FPointerEvent& TouchEvent) override;

	virtual FReply OnTouchMoved(const FGeometry& MyGeometry, const FPointerEvent& TouchEvent) override;

	virtual FReply OnTouchEnded(const FGeometry& MyGeometry, const FPointerEvent& TouchEvent) override;

private:

	void RegisterImGuiSettingsDelegates();
	void UnregisterImGuiSettingsDelegates();

	void SetHideMouseCursor(bool bHide);

	// Update visibility based on input state.
	void UpdateVisibility();

	// Update cursor based on input state.
	void UpdateMouseCursor();



	void UpdateCanvasControlMode(const FInputEvent& InputEvent);

	FVector2D TransformScreenPointToImGui(const FGeometry& MyGeometry, const FVector2D& Point) const;

	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyClippingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& WidgetStyle, bool bParentEnabled) const override;

	void SetImGuiTransform(const FSlateRenderTransform& Transform) { ImGuiTransform = Transform; }

	void OnDebugDraw();

	FImGuiModuleManager* ModuleManager = nullptr;
	TWeakObjectPtr<UImGuiInputHandler> InputHandler;

	FSlateRenderTransform ImGuiTransform;
	FSlateRenderTransform ImGuiRenderTransform;

	mutable TArray<FSlateVertex> VertexBuffer;
	mutable TArray<SlateIndex> IndexBuffer;

	bool bInputEnabled = true;
	bool bHideMouseCursor = true;

	TSharedPtr<SImGuiCanvasControl> CanvasControlWidget;
	TWeakPtr<SWidget> PreviousUserFocusedWidget;

    TUniquePtr<FImGuiContextProxy> ContextProxy;
};
