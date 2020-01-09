// Distributed under the MIT License (MIT) (see accompanying LICENSE file)

#pragma once

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
