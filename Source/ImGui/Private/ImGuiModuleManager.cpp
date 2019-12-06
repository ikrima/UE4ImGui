// Distributed under the MIT License (MIT) (see accompanying LICENSE file)

#include "ImGuiPrivatePCH.h"

#include "ImGuiModuleManager.h"

#include "ImGuiInteroperability.h"
#include "Utilities/WorldContextIndex.h"

#include <Modules/ModuleManager.h>

#include <imgui.h>
#include "Widgets/Docking/SDockTab.h"
#include "Framework/Docking/TabManager.h"

#if WITH_EDITORONLY_DATA
#include "LevelEditor.h"
#endif
#include "Widgets/SImGuiBaseWidget.h"


// High enough z-order guarantees that ImGui output is rendered on top of the game UI.
constexpr int32 IMGUI_WIDGET_Z_ORDER = 10000;


FImGuiModuleManager::FImGuiModuleManager()
	: Commands (Properties)
	, Settings (Properties, Commands)
	, ImGuiDemo(Properties)
{}


void FImGuiModuleManager::AddNewImGuiWindow(const UWorld& InWorld, const FString& InName, TUniquePtr<FImGuiDrawer> InImGuiDrawer)
{
    checkf(FSlateApplication::IsInitialized(), TEXT("Slate should be initialized before we can add widget to game viewports."));

    // Load & Build textures: Make sure that textures are loaded before the first Slate widget is created.
    {        
	    checkf(FSlateApplication::IsInitialized(), TEXT("Slate should be initialized before we can create textures."));

	    if (!bTexturesAndFontsLoaded)
	    {
            bTexturesAndFontsLoaded = true;

		    TextureManager.InitializeErrorTexture(FColor::Magenta);

		    // Create an empty texture at index 0. We will use it for ImGui outputs with null texture id.
		    TextureManager.CreatePlainTexture(FName{ "ImGuiModule_Plain" }, 2, 2, FColor::White);
	    }

        ContextManager.BuildFonts(TextureManager);
    }

    FTabManager* const tabManager = [] {
#if WITH_EDITOR
        if (FLevelEditorModule* levelEditorModule = FModuleManager::GetModulePtr<FLevelEditorModule>("LevelEditor"))
        {
            return levelEditorModule->GetLevelEditorTabManager().Get();
        }
#endif
        return (FTabManager*)(&FGlobalTabmanager::Get().Get());
    }();

    tabManager->InsertNewDocumentTab(
        TEXT("IMGUIWidget"),
        FTabManager::ESearchPreference::PreferLiveTab,
        SNew(SDockTab)
        .Label(NSLOCTEXT("IMGUIWidget", "IMGUIWidget", "IMGUIWidget"))
        .TabRole(ETabRole::DocumentTab)
        .ShouldAutosize(true)
        [
            SNew(SImGuiBaseWidget)
            .ModuleManager(this)
            .ContextName(InName)
            .ImGuiDrawer(InImGuiDrawer.Release())
            // To correctly clip borders. Using SScissorRectBox in older versions seems to be not necessary.
            .Clipping(EWidgetClipping::ClipToBounds)
        ]
    );
}
