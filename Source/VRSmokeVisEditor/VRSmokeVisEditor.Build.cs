// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;

public class VRSmokeVisEditor : ModuleRules
{
	public VRSmokeVisEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
        CppStandard = CppStandardVersion.Cpp17;

        PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
                "Core",
                "RenderCore",
                "Engine",
                "RHI",
                "AssetRegistry",
				"HeadMountedDisplay",
				"InputCore",
				"VRSmokeVis"
			}
            );
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
                "SlateCore",
                "UMG",
                "UnrealEd"
				// ... add private dependencies that you statically link with here ...	
			}
            );

		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);
	}
}
