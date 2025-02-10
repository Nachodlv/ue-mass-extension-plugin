#include "Fragments/MCMassCollisionTrait.h"

// UE Includes
#include "MassCommonFragments.h"
#include "MassEntityTemplateRegistry.h"
#include "MassMovementFragments.h"

// MC Includes
#include "Subsystems/MCWorldSubsystem.h"

void UMCMassCollisionTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const
{
	BuildContext.AddTag<FMCCollidesTag>();

	FMCCollisionsInformation& CollisionInfo = BuildContext.AddFragment_GetRef<FMCCollisionsInformation>();

	if (const UMCCollisionLayersSettings* LayerSettings = GetDefault<UMCCollisionLayersSettings>())
	{
		TOptional<int32> CollisionIndex = LayerSettings->GetCollisionIndexWithName(CollisionLayer);
		if (ensure(CollisionIndex))
		{
			CollisionInfo.CollisionLayerIndex = *CollisionIndex;
		}
	}

	BuildContext.RequireFragment<FAgentRadiusFragment>();
	BuildContext.RequireFragment<FTransformFragment>();
	BuildContext.RequireFragment<FMassVelocityFragment>();
}
