// Distributed under the MIT License (MIT) (see accompanying LICENSE file)

#include "UnrealImGui.h"
#if 0
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"

// We build ImGui source code as part of this module. This is for convenience (no need to manually build libraries for
// different target platforms) but it also exposes the whole ImGui source for inspection, which can be pretty handy.
// Source files are included from Third Party directory, so we can wrap them in required by Unreal Build System headers
// without modifications in ImGui source code.
//
// We don't need to define IMGUI_API manually because it is already done for this module.

#if PLATFORM_XBOXONE
// Disable Win32 functions used in ImGui and not supported on XBox.
#define IMGUI_DISABLE_WIN32_DEFAULT_CLIPBOARD_FUNCTIONS
#define IMGUI_DISABLE_WIN32_DEFAULT_IME_FUNCTIONS
#endif // PLATFORM_XBOXONE

#if PLATFORM_WINDOWS
#include <Windows/AllowWindowsPlatformTypes.h>
#endif // PLATFORM_WINDOWS

//THIRD_PARTY_INCLUDES_START
#include "imgui.cpp"
#include "imgui_demo.cpp"
#include "imgui_draw.cpp"
#include "imgui_widgets.cpp"
#include "misc/cpp/imgui_stdlib.cpp"
//THIRD_PARTY_INCLUDES_END 

#if PLATFORM_WINDOWS
#include <Windows/HideWindowsPlatformTypes.h>
#endif // PLATFORM_WINDOWS

#include "ImGuiInteroperability.h"


#undef IMGUI_DEFINE_MATH_OPERATORS
#endif