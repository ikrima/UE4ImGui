// Distributed under the MIT License (MIT) (see accompanying LICENSE file)

#pragma once

#include "ImGuiContextProxy.h"

enum class EIMThemeFont : uint8
{
  Default = 0,
  Roboto = 1,
  KarlaRegular = 2,
  CousineRegular = 3,
  DroidSans = 4,
  AdobeClean = 5,
};

constexpr uint8 EIMThemeFont_MaxCount = uint8(EIMThemeFont::AdobeClean) + 1;

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
