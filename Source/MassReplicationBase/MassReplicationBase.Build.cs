// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class MassReplicationBase : ModuleRules
{
	public MassReplicationBase(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"MassEntity",
			"MassCommon",
			"MassReplication",
			"MassSpawner",
			"NetCore",
		});

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"StructUtils",
			"DeveloperSettings" 
		});
	}
}