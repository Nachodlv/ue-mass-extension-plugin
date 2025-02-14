using UnrealBuildTool;

public class MassReplicationSmooth : ModuleRules
{
    public MassReplicationSmooth(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core"
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore",
                "NetCore",
                "MassReplicationBase",
                "MassEntity",
                "MassReplication",
                "MassRepresentation",
                "MassCommon",
                "MassSpawner",
                "StructUtils",
                "MassMovement",
                "MassActors"
            }
        );
    }
}