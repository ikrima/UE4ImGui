// Distributed under the MIT License (MIT) (see accompanying LICENSE file)

#pragma once

enum class EIMThemeFont : uint8
{
  Default        = 0,
  Roboto         = 1,
  KarlaRegular   = 2,
  CousineRegular = 3,
  DroidSans      = 4,
  AdobeClean     = 5,
};

constexpr uint8 EIMThemeFont_MaxCount = uint8(EIMThemeFont::AdobeClean) + 1;

struct UE4IMGUI_API FImGuiDrawer {
protected:
  virtual void OnInitialize() {}
  virtual void OnTick(const float InDeltaTime) {}
  virtual void OnDraw() = 0;
  virtual void OnDestroy() {}

public:
  virtual ~FImGuiDrawer()           = default;
  class FImGuiContextManager* imguiCtxMgr = nullptr;

  friend class FImGuiContextProxy;
};


// Manages ImGui context proxies.
class FImGuiContextManager
{
public:
  FImGuiContextManager()                            = default;
  FImGuiContextManager(const FImGuiContextManager&) = delete;
  FImGuiContextManager& operator=(const FImGuiContextManager&) = delete;
  FImGuiContextManager(FImGuiContextManager&&)                 = delete;
  FImGuiContextManager& operator=(FImGuiContextManager&&) = delete;

  struct ImFontAtlas&       GetFontAtlas() { return FontAtlas; }
  const struct ImFontAtlas& GetFontAtlas() const { return FontAtlas; }

  void           BuildFonts(class FTextureManager& TextureManager);
  struct ImFont* themeFonts[EIMThemeFont_MaxCount] = {};

  struct ImFontAtlas FontAtlas;
};
