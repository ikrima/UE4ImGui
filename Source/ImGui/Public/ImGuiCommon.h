// Copyright Kite & Lightning

#pragma once

#ifndef WITH_IMGUI
#define WITH_IMGUI 1
#endif

//#ifdef IMGUI_API
//#define WITH_IMGUI 1
//#else
//#define WITH_IMGUI 0
//#endif // IMGUI_API

#if WITH_IMGUI
#include "imgui.h"
#include "imgui_node_editor.h"
#endif 
