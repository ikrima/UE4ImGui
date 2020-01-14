// Distributed under the MIT License (MIT) (see accompanying LICENSE file)

#pragma once

#include "ImGuiModuleProperties.h"
#include "ImGuiTextureHandle.h"
#include "Templates/UniquePtr.h"
#include <Modules/ModuleManager.h>
#include "ImGuiDrawer.h"


class FImGuiModule : public IModuleInterface
{
public:

	/**
	 * Singleton-like access to this module's interface. This is just for convenience!
	 * Beware of calling this during the shutdown phase, though. Your module might have been unloaded already.
	 *
	 * @return Returns singleton instance, loading the module on demand if needed
	 */
	static inline FImGuiModule& Get()
	{
		return FModuleManager::GetModuleChecked<FImGuiModule>("ImGui");
	}

	/**
	 * Checks to see if this module is loaded and ready. It is only valid to call Get() if IsAvailable() returns true.
	 *
	 * @return True if the module is loaded and ready to use
	 */
	static inline bool IsAvailable()
	{
		return FModuleManager::Get().IsModuleLoaded("ImGui");
	}

    IMGUI_API virtual void AddNewImGuiWindow(const UWorld& InWorld, const FString& InName, TUniquePtr<FImGuiDrawer> InImGuiDrawer);

	/**
	 * If it exists, get a handle to the texture with given resource name.
	 *
	 * @param Name - Resource name of a texture to find
	 * @returns Handle to a registered texture or invalid handle if resources could not be found or were not valid
	 */
	virtual FImGuiTextureHandle FindTextureHandle(const FName& Name);

	/**
	 * Register texture and create its Slate resources. If texture with that name already exists then it may be updated
	 * or if bMakeUnique is true, exception will be thrown. Throws exception, if name argument is NAME_None or texture
	 * is null.
	 *
	 * Note, that updating texture resources doesn't invalidate already existing handles and returned handle will have
	 * the same value.
	 *
	 * @param Name - Resource name for the texture that needs to be registered or updated
	 * @param Texture - Texture for which we want to create or update Slate resources
	 * @param bMakeUnique - If false then existing resources are updated/overwritten (default). If true, then stricter
	 *     policy is applied and if resource with that name exists then exception is thrown.
	 * @returns Handle to the texture resources, which can be used to release allocated resources and as an argument to
	 *     relevant ImGui functions
	 */
	virtual FImGuiTextureHandle RegisterTexture(const FName& Name, class UTexture2D* Texture, bool bMakeUnique = false);

	/**
	 * Unregister texture and release its Slate resources. If handle is null or not valid, this function fails silently
	 * (for definition of 'valid' look @ FImGuiTextureHandle).
	 *
	 * @returns ImGui Texture Handle to texture that needs to be unregistered
	 */
	virtual void ReleaseTexture(const FImGuiTextureHandle& Handle);

	/**
	 * Get ImGui module properties.
	 *
	 * @returns Reference to an instance of ImGui module properties that allows to read and/or modify module state.
	 */
	virtual FImGuiModuleProperties& GetProperties();
	virtual const FImGuiModuleProperties& GetProperties() const;

	/**
	 * DEPRECIATED: Please use GetProperties() as this function is scheduled for removal.
	 * Check whether Input Mode is enabled (tests ImGui.InputEnabled console variable).
	 *
	 * @returns True, if Input Mode is enabled (ImGui.InputEnabled != 0) and false otherwise.
	 */
	virtual bool IsInputMode() const;

	/**
	 * DEPRECIATED: Please use GetProperties() as this function is scheduled for removal.
	 * Set Input Mode state (sets ImGui.InputEnabled console variable, so it can be used together with a console).
	 *
	 * @param bEnabled - Whether Input Mode should be enabled (ImGui.InputEnabled = 1) or not (ImGui.InputEnabled = 0).
	 */
	virtual void SetInputMode(bool bEnabled);

	/**
	 * DEPRECIATED: Please use GetProperties() as this function is scheduled for removal.
	 * Toggle Input Mode state (changes ImGui.InputEnabled console variable).
	 */
	virtual void ToggleInputMode();

	/**
	 * DEPRECIATED: Please use GetProperties() as this function is scheduled for removal.
	 * Check whether ImGui Demo is shown (tests ImGui.ShowDemo console variable).
	 *
	 * @returns True, if demo is shown (ImGui.ShowDemo != 0) and false otherwise.
	 */
	virtual bool IsShowingDemo() const;

	/**
	 * DEPRECIATED: Please use GetProperties() as this function is scheduled for removal.
	 * Set whether to show ImGui Demo (sets ImGui.ShowDemo console variable, so it can be used together with a console).
	 *
	 * @param bShow - Whether to show ImGui Demo (ImGui.ShowDemo = 1) or not (ImGui.ShowDemo = 0).
	 */
	virtual void SetShowDemo(bool bShow);

	/**
	 * DEPRECIATED: Please use GetProperties() as this function is scheduled for removal.
	 * Toggle ImGui Demo (changes ImGui.ShowDemo console variable).
	 */
	virtual void ToggleShowDemo();

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	private:

#if WITH_EDITOR
	virtual void SetProperties(const FImGuiModuleProperties& Properties);
#endif
};
