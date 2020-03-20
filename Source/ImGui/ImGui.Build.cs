// Distributed under the MIT License (MIT) (see accompanying LICENSE file)

using System.Collections.Generic;
using System.IO;
using UnrealBuildTool;

public class ImGui : ModuleRules
{
	public ImGui(ReadOnlyTargetRules Target) : base(Target)
	{
		bool bBuildEditor = Target.bBuildEditor;

		// Developer modules are automatically loaded only in editor builds but can be stripped out from other builds.
		// Enable runtime loader, if you want this module to be automatically loaded in runtime builds (monolithic).
		bool bEnableRuntimeLoader = true;

        bEnforceIWYU = true;
		PCHUsage     = PCHUsageMode.UseExplicitOrSharedPCHs;
        bFasterWithoutUnity = true;
        bEnforceIWYU = true;
        bLegacyPublicIncludePaths = false;

        PrivatePCHHeaderFile = "Private/ImGuiPrivatePCH.h";

        PublicIncludePaths.AddRange(
			new string[] {
                Path.Combine(ModuleDirectory, "Public"),
                Path.Combine(ModuleDirectory, "ThirdParty/ImGuiLibrary/Include"),
				Path.Combine(ModuleDirectory, "ThirdParty/ImGuiLibrary/Include/misc/cpp"),
                Path.Combine(ModuleDirectory, "ThirdParty/ImGuiLibrary/Include/misc/fonts"),
                Path.Combine(ModuleDirectory, "ThirdParty/ImGuiLibrary/Include/misc/freetype"),
                Path.Combine(ModuleDirectory, "ThirdParty/IconFontCppHeaders/Public"),
                Path.Combine(ModuleDirectory, "ThirdParty/ImGuiAlFonts/Public"),
                Path.Combine(ModuleDirectory, "ThirdParty/ImGuiNodeEditor/Include"),
                Path.Combine(ModuleDirectory, "ThirdParty/NelariusImNodes/Public"),
				Path.Combine(ModuleDirectory, "ThirdParty/ImGuiColorTextEdit/Public"),
			}
			);


		PrivateIncludePaths.AddRange(
			new string[] {
				Path.Combine(ModuleDirectory, "Private"),
				Path.Combine(ModuleDirectory, "ThirdParty/ImGuiLibrary/Private"),
                Path.Combine(ModuleDirectory, "ThirdParty/ImGuiLibrary/Private/misc/cpp"),
                Path.Combine(ModuleDirectory, "ThirdParty/ImGuiLibrary/Private/misc/fonts"),
				Path.Combine(ModuleDirectory, "ThirdParty/ImGuiLibrary/Private/misc/freetype"),
                Path.Combine(ModuleDirectory, "ImGui/ThirdParty/ImGuiNodeEditor/Private"),
                Path.Combine(ModuleDirectory, "ImGui/ThirdParty/NelariusImNodes/Private"),
				Path.Combine(ModuleDirectory, "ImGui/ThirdParty/ImGuiColorTextEdit/Private"),
			}
			);


		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"Projects"
				// ... add other public dependencies that you statically link with here ...
            }
			);


		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"InputCore",
				"Slate",
				"SlateCore",
				// ... add private dependencies that you statically link with here ...	
                
                "Projects",
            }
			);


		if (bBuildEditor)
		{
			PrivateDependencyModuleNames.AddRange(
				new string[]
				{
					"EditorStyle",
					"Settings",
					"UnrealEd",
				}
				);
		}


		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);

		PrivateDefinitions.Add(string.Format("RUNTIME_LOADER_ENABLED={0}", bEnableRuntimeLoader ? 1 : 0));
	}
}
