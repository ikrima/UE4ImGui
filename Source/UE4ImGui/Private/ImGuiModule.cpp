// Distributed under the MIT License (MIT) (see accompanying LICENSE file)

#include "ImGuiModule.h"
#include "UnrealImGui.h"

#include "ImGuiModuleManager.h"

#include "ImGuiTextureHandle.h"
#include "TextureManager.h"
#include "Utilities/WorldContext.h"
#include "Utilities/WorldContextIndex.h"

#if WITH_EDITOR
#include "Editor/ImGuiEditor.h"
#endif

#include <Interfaces/IPluginManager.h>
#include "GenericPlatform/GenericPlatformFile.h"


#define LOCTEXT_NAMESPACE "FImGuiModule"


struct EDelegateCategory
{
	enum
	{
		// Default per-context draw events.
		Default,

		// Multi-context draw event defined in context manager.
		MultiContext
	};
};

static FImGuiModuleManager* ImGuiModuleManager = nullptr;

#if WITH_EDITOR
static FImGuiEditor* ImGuiEditor = nullptr;
#endif



FString FImGuiModule::GetImGuiIniDir()
{
	static const FString SaveDirectory = [] {
		const FString SavedDir = FPaths::ProjectSavedDir();
		FString       Directory = SavedDir / FImGuiModule::PluginName;
		// Make sure that directory is created.
		IPlatformFile::GetPlatformPhysical().CreateDirectory(*Directory);
		return Directory;
	}();
	return SaveDirectory;
}

FString FImGuiModule::GetImGuiPathForIni(const FString& Name)
{
	return FPaths::Combine(GetImGuiIniDir(), Name);
}


void FImGuiModule::AddNewImGuiWindow(const FString& InName, TUniquePtr<FImGuiDrawer> InImGuiDrawer)
{
    ImGuiModuleManager->AddNewImGuiWindow(InName, MoveTemp(InImGuiDrawer));
}

FImGuiTextureHandle FImGuiModule::FindTextureHandle(const FName& Name)
{
	const TextureIndex Index = ImGuiModuleManager->GetTextureManager().FindTextureIndex(Name);
	return (Index != INDEX_NONE) ? FImGuiTextureHandle{ Name, ImGuiInterops::ToImTextureID(Index) } : FImGuiTextureHandle{};
}

FImGuiTextureHandle FImGuiModule::RegisterTexture(const FName& Name, class UTexture2D* Texture, bool bMakeUnique)
{
	const TextureIndex Index = ImGuiModuleManager->GetTextureManager().CreateTextureResources(Name, Texture, bMakeUnique);
	return FImGuiTextureHandle{ Name, ImGuiInterops::ToImTextureID(Index) };
}

void FImGuiModule::ReleaseTexture(const FImGuiTextureHandle& Handle)
{
	if (Handle.IsValid())
	{
		ImGuiModuleManager->GetTextureManager().ReleaseTextureResources(ImGuiInterops::ToTextureIndex(Handle.GetTextureId()));
	}
}

void FImGuiModule::StartupModule()
{
	// Create managers that implements module logic.

	checkf(!ImGuiModuleManager, TEXT("Instance of the ImGui Module Manager already exists. Instance should be created only during module startup."));
	ImGuiModuleManager = new FImGuiModuleManager();

#if WITH_EDITOR
	checkf(!ImGuiEditor, TEXT("Instance of the ImGui Editor already exists. Instance should be created only during module startup."));
	ImGuiEditor = new FImGuiEditor();
#endif
}

void FImGuiModule::ShutdownModule()
{
	// In editor store data that we want to move to hot-reloaded module.

#if WITH_EDITOR
	static bool bMoveProperties = true;
	static FImGuiModuleProperties PropertiesToMove = ImGuiModuleManager->GetProperties();
#endif

	// Before we shutdown we need to delete managers that will do all the necessary cleanup.

#if WITH_EDITOR
	checkf(ImGuiEditor, TEXT("Null ImGui Editor. ImGui editor instance should be deleted during module shutdown."));
	delete ImGuiEditor;
	ImGuiEditor = nullptr;
#endif

	checkf(ImGuiModuleManager, TEXT("Null ImGui Module Manager. Module manager instance should be deleted during module shutdown."));
	delete ImGuiModuleManager;
	ImGuiModuleManager = nullptr;
}

#if WITH_EDITOR
void FImGuiModule::SetProperties(const FImGuiModuleProperties& Properties)
{
	ImGuiModuleManager->GetProperties() = Properties;
}
#endif

FImGuiModuleProperties& FImGuiModule::GetProperties()
{
	return ImGuiModuleManager->GetProperties();
}

const FImGuiModuleProperties& FImGuiModule::GetProperties() const
{
	return ImGuiModuleManager->GetProperties();
}

bool FImGuiModule::IsInputMode() const
{
	return ImGuiModuleManager && ImGuiModuleManager->GetProperties().IsInputEnabled();
}

void FImGuiModule::SetInputMode(bool bEnabled)
{
	if (ImGuiModuleManager)
	{
		ImGuiModuleManager->GetProperties().SetInputEnabled(bEnabled);
	}
}

void FImGuiModule::ToggleInputMode()
{
	if (ImGuiModuleManager)
	{
		ImGuiModuleManager->GetProperties().ToggleInput();
	}
}

bool FImGuiModule::IsShowingDemo() const
{
	return ImGuiModuleManager && ImGuiModuleManager->GetProperties().ShowDemo();
}

void FImGuiModule::SetShowDemo(bool bShow)
{
	if (ImGuiModuleManager)
	{
		ImGuiModuleManager->GetProperties().SetShowDemo(bShow);
	}
}

void FImGuiModule::ToggleShowDemo()
{
	if (ImGuiModuleManager)
	{
		ImGuiModuleManager->GetProperties().ToggleDemo();
	}
}


//----------------------------------------------------------------------------------------------------
// Runtime loader
//----------------------------------------------------------------------------------------------------

#if !WITH_EDITOR && RUNTIME_LOADER_ENABLED

class FImGuiModuleLoader
{
	FImGuiModuleLoader()
	{
		if (!Load())
		{
			FModuleManager::Get().OnModulesChanged().AddRaw(this, &FImGuiModuleLoader::LoadAndRelease);
		}
	}

	// For different engine versions.
	static FORCEINLINE bool IsValid(const TSharedPtr<IModuleInterface>& Ptr) { return Ptr.IsValid(); }
	static FORCEINLINE bool IsValid(const IModuleInterface* const Ptr) { return Ptr != nullptr; }

	bool Load()
	{
		return IsValid(FModuleManager::Get().LoadModule(ModuleName));
	}

	void LoadAndRelease(FName Name, EModuleChangeReason Reason)
	{
		// Avoid handling own load event.
		if (Name != ModuleName)
		{
			// Try loading until success and then release.
			if (Load())
			{
				FModuleManager::Get().OnModulesChanged().RemoveAll(this);
			}
		}
	}

	static FName ModuleName;

	static FImGuiModuleLoader Instance;
};

FName FImGuiModuleLoader::ModuleName = FImGuiModule::ModuleName;

// In monolithic builds this will start loading process.
FImGuiModuleLoader FImGuiModuleLoader::Instance;

#endif // !WITH_EDITOR && RUNTIME_LOADER_ENABLED


//----------------------------------------------------------------------------------------------------
// Partial implementations of other classes that needs access to ImGuiModuleManager
//----------------------------------------------------------------------------------------------------

bool FImGuiTextureHandle::HasValidEntry() const
{
	const TextureIndex Index = ImGuiInterops::ToTextureIndex(TextureId);
	return Index != INDEX_NONE && ImGuiModuleManager && ImGuiModuleManager->GetTextureManager().GetTextureName(Index) == Name;
}


#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FImGuiModule, UE4ImGui)
