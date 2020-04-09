// Distributed under the MIT License (MIT) (see accompanying LICENSE file)


#include "ImGuiContextManager.h"

#include "UnrealImGui.h"
#include "Utilities/ScopeGuards.h"
#include "Utilities/WorldContext.h"
#include "Utilities/WorldContextIndex.h"

#include "IconFontCppHeaders/IconsFontAwesome5.h"
#include "fonts/CousineRegular.inl"
#include "fonts/KarlaRegular.inl"
#include "fonts/GoogleMaterialDesign.inl"
#include "fonts/FontAwesome5Solid900.inl"
#include "Interfaces/IPluginManager.h"
#include "TextureManager.h"
#include "Misc/Paths.h"
#include "ImGuiModule.h"

void FImGuiContextManager::BuildFonts(FTextureManager& TextureManager)
{    
    constexpr float fontSize = 16;

    auto addIconFont = [this,fontSize] {
	    ImGuiIO& io = ImGui::GetIO();
	    static const ImWchar icons_ranges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };
	    ImFontConfig icons_config;
	    // merge in icons from Font Awesome
	    icons_config.MergeMode = true;
	    icons_config.PixelSnapH = true;
	    icons_config.OversampleH = 2;
	    icons_config.OversampleV = 1;
	    icons_config.GlyphOffset.y += 1.0f;
	    icons_config.OversampleH = icons_config.OversampleV = 1;
	    icons_config.PixelSnapH = true;
	    icons_config.SizePixels = 13.0f * 1.0f;

        FontAtlas.AddFontFromMemoryCompressedTTF(FontAwesome5Solid900_compressed_data, FontAwesome5Solid900_compressed_size, fontSize, &icons_config, icons_ranges);
    };

    // Create a font atlas texture.
    ImFontConfig icons_config;
    icons_config.MergeMode = false;
    icons_config.PixelSnapH = true;
    icons_config.OversampleH = 2;
    icons_config.OversampleV = 1;
    icons_config.OversampleH = icons_config.OversampleV = 1;
    icons_config.PixelSnapH = true;

    const FString robotoFontPath = FPaths::ConvertRelativePathToFull(FImGuiModule::GetImGuiFontDir() / TEXT("Roboto-Medium.ttf"));
    themeFonts[uint8(EIMThemeFont::Default)] = FontAtlas.AddFontDefault();
    themeFonts[uint8(EIMThemeFont::Roboto)] = FontAtlas.AddFontFromFileTTF(StringCast<ANSICHAR>(*robotoFontPath).Get(), fontSize, &icons_config);
    addIconFont();
    strncpy_s(icons_config.Name, "KarlaRegular", sizeof(icons_config.Name));
    themeFonts[uint8(EIMThemeFont::KarlaRegular)] = FontAtlas.AddFontFromMemoryCompressedTTF(KarlaRegular_compressed_data, KarlaRegular_compressed_size, fontSize, &icons_config);
    addIconFont();
    strncpy_s(icons_config.Name, "CousineRegular", sizeof(icons_config.Name));
    themeFonts[uint8(EIMThemeFont::CousineRegular)] = FontAtlas.AddFontFromMemoryCompressedTTF(CousineRegular_compressed_data, CousineRegular_compressed_size, fontSize, &icons_config);
    addIconFont();
    strncpy_s(icons_config.Name, "DroidSans", sizeof(icons_config.Name));
    const FString droidSansFontPath = FPaths::ConvertRelativePathToFull(FImGuiModule::GetImGuiFontDir() / TEXT("DroidSans.ttf"));
    themeFonts[uint8(EIMThemeFont::DroidSans)] = FontAtlas.AddFontFromFileTTF(StringCast<ANSICHAR>(*droidSansFontPath).Get(), fontSize, &icons_config);
    addIconFont();
    strncpy_s(icons_config.Name, "AdobeClean", sizeof(icons_config.Name));
    const FString adobeCleanFontPath = FPaths::ConvertRelativePathToFull(FImGuiModule::GetImGuiFontDir() / TEXT("AdobeClean-Regular.ttf"));
    themeFonts[uint8(EIMThemeFont::AdobeClean)] = FontAtlas.AddFontFromFileTTF(StringCast<ANSICHAR>(*adobeCleanFontPath).Get(), fontSize, &icons_config);
    addIconFont();

	unsigned char* Pixels;
	int Width, Height, Bpp;
    FontAtlas.GetTexDataAsRGBA32(&Pixels, &Width, &Height, &Bpp);

	TextureIndex FontsTexureIndex = TextureManager.CreateTexture(FName{ "ImGuiModule_FontAtlas" }, Width, Height, Bpp, Pixels);

	// Set font texture index in ImGui.
    FontAtlas.TexID = ImGuiInterops::ToImTextureID(FontsTexureIndex);
}
