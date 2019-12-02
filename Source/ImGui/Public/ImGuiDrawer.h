// Distributed under the MIT License (MIT) (see accompanying LICENSE file)

#pragma once

#include <Core.h>

struct IMGUI_API FImGuiDrawer
{
protected:
    virtual void OnInitialize() {}
    virtual void OnDestroy() {}
    virtual void OnDraw() = 0;

public:
    virtual ~FImGuiDrawer() = default;

    friend class FImGuiContextProxy;
};
