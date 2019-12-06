// Distributed under the MIT License (MIT) (see accompanying LICENSE file)
#pragma  once

// Module-wide macros
#include <Runtime/Launch/Resources/Version.h>
#include <Core.h>
#include <Engine.h>
#include <Logging/LogMacros.h>

// For backward compatibility we will use FStringClassReference which in newer engine versions is a typedef for
// FSoftClassPath. Include right soft class reference header to avoid warnings in newer engine version.
#include <UObject/SoftObjectPath.h>
#include "UnrealImGui.h"