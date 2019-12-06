// Distributed under the MIT License (MIT) (see accompanying LICENSE file)

#pragma once

#include "ImGuiContextProxy.h"
#include "ImGuiTheming.h"

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
