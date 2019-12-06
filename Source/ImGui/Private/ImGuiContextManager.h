// Distributed under the MIT License (MIT) (see accompanying LICENSE file)

#pragma once

#include "ImGuiContextProxy.h"
#include "ImGuiTheming.h"


// TODO: It might be useful to broadcast FContextProxyCreatedDelegate to users, to support similar cases to our ImGui
// demo, but we would need to remove from that interface internal classes.

// Delegate called when new context proxy is created.
// @param ContextIndex - Index for that world
// @param ContextProxy - Created context proxy
DECLARE_MULTICAST_DELEGATE_TwoParams(FContextProxyCreatedDelegate, int32, FImGuiContextProxy&);

// Manages ImGui context proxies.
class FImGuiContextManager
{
public:
    FImGuiContextManager() = default;
	FImGuiContextManager(const FImGuiContextManager&) = delete;
	FImGuiContextManager& operator=(const FImGuiContextManager&) = delete;
	FImGuiContextManager(FImGuiContextManager&&) = delete;
	FImGuiContextManager& operator=(FImGuiContextManager&&) = delete;

	ImFontAtlas& GetFontAtlas() { return FontAtlas; }
	const ImFontAtlas& GetFontAtlas() const { return FontAtlas; }

    void BuildFonts(class FTextureManager& TextureManager);
    struct ImFont* themeFonts[EIMThemeFont_MaxCount] = {};

private:

	ImFontAtlas FontAtlas;
};
