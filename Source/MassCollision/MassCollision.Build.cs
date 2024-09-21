using UnrealBuildTool;

public class MassCollision : ModuleRules
{
    public MassCollision(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core", 
                "GeometryCore", 
                "MassEntity", 
                "MassSpawner",
                "MassCommon",
                "MassMovement"
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore", 
            }
        );
    }
}