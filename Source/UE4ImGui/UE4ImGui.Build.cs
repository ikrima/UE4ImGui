// Distributed under the MIT License (MIT) (see accompanying LICENSE file)

using System.Collections.Generic;
using System.IO;
using UnrealBuildTool;

public class UE4ImGui : ModuleRules
{
    const bool bUseFreetype = false;
    public UE4ImGui(ReadOnlyTargetRules Target) : base(Target)
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
        CppStandard = CppStandardVersion.Latest;

        PrivatePCHHeaderFile = "Private/ImGuiPrivatePCH.h";

        PublicIncludePaths.AddRange(
            new string[] {
                Path.Combine(ModuleDirectory, "Public"),
                Path.Combine(ModuleDirectory, "ThirdParty/ImGui"),
                Path.Combine(ModuleDirectory, "ThirdParty/ImGui/ImGuiUX"),
                Path.Combine(ModuleDirectory, "ThirdParty/ImGui/ImGuiUX/IconFontCppHeaders"),
                Path.Combine(ModuleDirectory, "ThirdParty/ImGui/ImGuiUX/ImGuiAl"),
                Path.Combine(ModuleDirectory, "ThirdParty/ImGui/ImGuiUX/ImGuiColorTextEdit"),
                Path.Combine(ModuleDirectory, "ThirdParty/ImGui/ImGuiUX/Nelarius_ImNodes"),
                Path.Combine(ModuleDirectory, "ThirdParty/ImGui/ImGuiUX/Dmd_NodeEditor/NodeEditor/Include"),
                Path.Combine(ModuleDirectory, "ThirdParty/ImGui/ImGuiUX/ImGuizmo"),
            }
            );


        PrivateIncludePaths.AddRange(
            new string[] {
                Path.Combine(ModuleDirectory, "Private"),
                Path.Combine(ModuleDirectory, "ThirdParty/ImGui"),
                Path.Combine(ModuleDirectory, "ThirdParty/ImGui/misc/cpp"),
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

        if (bUseFreetype)
        {
            AddEngineThirdPartyPrivateStaticDependencies(Target, "FreeType2");
            PublicDefinitions.Add("WITH_IMGUI_FREETYPE=1");
        }
        else
        {
            PublicDefinitions.Add("WITH_IMGUI_FREETYPE=0");
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
