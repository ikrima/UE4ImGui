// Distributed under the MIT License (MIT) (see accompanying LICENSE file)


#include "ImGuiModuleSettings.h"
#include "UnrealImGui.h"

#include "ImGuiModuleCommands.h"
#include "ImGuiModuleProperties.h"


//====================================================================================================
// UImGuiSettings
//====================================================================================================

FSimpleMulticastDelegate UImGuiSettings::OnSettingsLoaded;

UImGuiSettings::UImGuiSettings()
{
    CategoryName = TEXT("Plugins");
    SectionName = FImGuiModule::PluginName;
}

void UImGuiSettings::PostInitProperties()
{
	Super::PostInitProperties();

#if WITH_EDITOR
    if (IsTemplate())
    {
        ImportConsoleVariableValues();
    }
#endif // #if WITH_EDITOR

	if (IsTemplate())
	{
		OnSettingsLoaded.Broadcast();
	}
}

#if WITH_EDITOR
void UImGuiSettings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);

    if (PropertyChangedEvent.Property)
    {
        ExportValuesToConsoleVariables(PropertyChangedEvent.Property);
    }
}
#endif // #if WITH_EDITOR

//====================================================================================================
// FImGuiModuleSettings
//====================================================================================================

FImGuiModuleSettings::FImGuiModuleSettings(FImGuiModuleProperties& InProperties, FImGuiModuleCommands& InCommands)
	: Properties(InProperties)
	, Commands(InCommands)
{
#if WITH_EDITOR
	FCoreUObjectDelegates::OnObjectPropertyChanged.AddRaw(this, &FImGuiModuleSettings::OnPropertyChanged);
#endif
	UImGuiSettings::OnSettingsLoaded.AddRaw(this, &FImGuiModuleSettings::UpdateSettings);

	// Call initializer to support settings already loaded (editor).
	UpdateSettings();
}

FImGuiModuleSettings::~FImGuiModuleSettings()
{

	UImGuiSettings::OnSettingsLoaded.RemoveAll(this);

#if WITH_EDITOR
	FCoreUObjectDelegates::OnObjectPropertyChanged.RemoveAll(this);
#endif
}

void FImGuiModuleSettings::UpdateSettings()
{
	if (UImGuiSettings const* SettingsObject = GetDefault<UImGuiSettings>())
	{
		SetImGuiInputHandlerClass(SettingsObject->ImGuiInputHandlerClass);
		SetShareKeyboardInput(SettingsObject->bShareKeyboardInput);
		SetShareGamepadInput(SettingsObject->bShareGamepadInput);
		SetShareMouseInput(SettingsObject->bShareMouseInput);
		SetUseSoftwareCursor(SettingsObject->bUseSoftwareCursor);
		SetToggleInputKey(SettingsObject->ToggleInput);
	}
}

void FImGuiModuleSettings::SetImGuiInputHandlerClass(const FStringClassReference& ClassReference)
{
	if (ImGuiInputHandlerClass != ClassReference)
	{
		ImGuiInputHandlerClass = ClassReference;
		OnImGuiInputHandlerClassChanged.Broadcast(ClassReference);
	}
}

void FImGuiModuleSettings::SetShareKeyboardInput(bool bShare)
{
	if (bShareKeyboardInput != bShare)
	{
		bShareKeyboardInput = bShare;
		Properties.SetKeyboardInputShared(bShare);
	}
}

void FImGuiModuleSettings::SetShareGamepadInput(bool bShare)
{
	if (bShareGamepadInput != bShare)
	{
		bShareGamepadInput = bShare;
		Properties.SetGamepadInputShared(bShare);
	}
}

void FImGuiModuleSettings::SetShareMouseInput(bool bShare)
{
	if (bShareMouseInput != bShare)
	{
		bShareMouseInput = bShare;
		Properties.SetMouseInputShared(bShare);
	}
}

void FImGuiModuleSettings::SetUseSoftwareCursor(bool bUse)
{
	if (bUseSoftwareCursor != bUse)
	{
		bUseSoftwareCursor = bUse;
		OnUseSoftwareCursorChanged.Broadcast(bUse);
	}
}

void FImGuiModuleSettings::SetToggleInputKey(const FImGuiKeyInfo& KeyInfo)
{
	if (ToggleInputKey != KeyInfo)
	{
		ToggleInputKey = KeyInfo;
		Commands.SetKeyBinding(FImGuiModuleCommands::ToggleInput, ToggleInputKey);
	}
}

#if WITH_EDITOR

void FImGuiModuleSettings::OnPropertyChanged(class UObject* ObjectBeingModified, struct FPropertyChangedEvent& PropertyChangedEvent)
{
	if (ObjectBeingModified == GetDefault<UImGuiSettings>())
	{
		UpdateSettings();
	}
}

#endif // WITH_EDITOR
