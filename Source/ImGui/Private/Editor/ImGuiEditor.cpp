// Distributed under the MIT License (MIT) (see accompanying LICENSE file)


#if WITH_EDITOR
#include "ImGuiEditor.h"

#include "UnrealImGui.h"

#include "ImGuiKeyInfoCustomization.h"
#include "ImGuiModuleSettings.h"

#include <ISettingsModule.h>
#include "Modules/ModuleManager.h"


#define LOCTEXT_NAMESPACE "ImGuiEditor"

#define SETTINGS_CONTAINER TEXT("Project"), TEXT("Plugins"), TEXT("ImGui")


namespace
{
	ISettingsModule* GetSettingsModule()
	{
		return FModuleManager::GetModulePtr<ISettingsModule>("Settings");
	}

	FPropertyEditorModule* GetPropertyEditorModule()
	{
		return FModuleManager::GetModulePtr<FPropertyEditorModule>("PropertyEditor");
	}
}

FImGuiEditor::FImGuiEditor()
{
	Register();

	// As a side effect of being part of the ImGui module, we need to support deferred registration (only executed if
	// module is loaded manually at the very early stage).
	if (!IsRegistrationCompleted())
	{
		CreateRegistrator();
	}
}

FImGuiEditor::~FImGuiEditor()
{
	Unregister();
}

void FImGuiEditor::Register()
{
	if (!bCustomPropertyTypeLayoutsRegistered)
	{
		if (FPropertyEditorModule* PropertyModule = GetPropertyEditorModule())
		{
			bCustomPropertyTypeLayoutsRegistered = true;

			PropertyModule->RegisterCustomPropertyTypeLayout("ImGuiKeyInfo",
				FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FImGuiKeyInfoCustomization::MakeInstance));
		}
	}
}

void FImGuiEditor::Unregister()
{
	if (bCustomPropertyTypeLayoutsRegistered)
	{
		bCustomPropertyTypeLayoutsRegistered = false;

		if (FPropertyEditorModule* PropertyModule = GetPropertyEditorModule())
		{
			PropertyModule->UnregisterCustomPropertyTypeLayout("ImGuiKeyInfo");
		}
	}
}

void FImGuiEditor::CreateRegistrator()
{
	if (!RegistratorHandle.IsValid())
	{
		RegistratorHandle = FModuleManager::Get().OnModulesChanged().AddLambda([this](FName Name, EModuleChangeReason Reason)
		{
			if (Reason == EModuleChangeReason::ModuleLoaded)
			{
				Register();
			}

			if (IsRegistrationCompleted())
			{
				ReleaseRegistrator();
			}
		});
	}
}

void FImGuiEditor::ReleaseRegistrator()
{
	if (RegistratorHandle.IsValid())
	{
		FModuleManager::Get().OnModulesChanged().Remove(RegistratorHandle);
		RegistratorHandle.Reset();
	}
}


#undef SETTINGS_CONTAINER
#undef LOCTEXT_NAMESPACE

#endif // WITH_EDITOR
