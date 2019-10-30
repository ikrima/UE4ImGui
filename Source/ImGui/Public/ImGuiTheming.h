// Distributed under the MIT License (MIT) (see accompanying LICENSE file)
#pragma once

#include "ImGuiTheming.generated.h"

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

USTRUCT()
struct IMGUI_API FImGuiThemeStyle
{
    GENERATED_BODY()

    void OnAttach();
    void OnDetach();

    static void SetTheme(EIMTheme);

    EIMTheme theme    = EIMTheme::Dark;
    float    fontSize = 16.0f;
private:
    void SetImGuiStyle();
    void AddIconFont();

};