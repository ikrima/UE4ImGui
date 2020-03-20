// Distributed under the MIT License (MIT) (see accompanying LICENSE file)

#pragma once

struct UE4IMGUI_API FImGuiDrawer
{
protected:
    virtual void OnInitialize() {}
    virtual void OnTick(const float InDeltaTime) {}
    virtual void OnDraw() = 0;
    virtual void OnDestroy() {}

public:
    virtual ~FImGuiDrawer() = default;

    friend class FImGuiContextProxy;
};
