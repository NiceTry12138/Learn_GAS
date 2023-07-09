// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class GasLearn : ModuleRules
{
	public GasLearn(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { 
			"Core", "CoreUObject", "Engine", "InputCore", "HeadMountedDisplay", "EnhancedInput", 
			"GameplayAbilities", 
			"GameplayTasks", 
		});
	}
}
