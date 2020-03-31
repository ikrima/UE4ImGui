// Distributed under the MIT License (MIT) (see accompanying LICENSE file)
#pragma once

#include "ImGuiTheming.generated.h"

struct ImFontAtlas;

enum class EIMTheme : uint8
{
  Black,
  Dark,
  Grey,
  Light,
  Blue,
  ClassicLight,
  ClassicDark,
  Classic,
  Cherry
};

enum class EIMThemeFont : uint8
{
  Default        = 0,
  Roboto         = 1,
  KarlaRegular   = 2,
  CousineRegular = 3,
  DroidSans      = 4
};
constexpr uint8 EIMThemeFont_MaxCount = uint8(EIMThemeFont::DroidSans) + 1;

USTRUCT()
struct IMGUI_API FImGuiThemeStyle {
  GENERATED_BODY()

  EIMTheme     theme     = EIMTheme::Dark;
  EIMThemeFont themeFont = EIMThemeFont::DroidSans;
  float        fontSize  = 13.0f;
  ImFontAtlas* fontAtlas = nullptr;

  void        OnInit(ImFontAtlas& InFontAtlas);
  void        OnBegin();
  void        OnEnd();
  void        OnDestroy();
  void        SetImGuiStyle();
  static void SetTheme(EIMTheme);
};