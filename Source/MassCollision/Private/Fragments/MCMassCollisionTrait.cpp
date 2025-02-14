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
	
	BuildContext.AddFragment<FMCCollisionsInformation>();

	FMassEntityManager& EntityManager = UE::Mass::Utils::GetEntityManagerChecked(World);
	FMCCollisionLayer CollisionLayerFragment;
	
	if (const UMCCollisionLayersSettings* LayerSettings = GetDefault<UMCCollisionLayersSettings>())
	{
		TOptional<int32> CollisionIndex = LayerSettings->GetCollisionIndexWithName(CollisionLayer);
		if (ensure(CollisionIndex))
		{
			CollisionLayerFragment.CollisionLayerIndex = *CollisionIndex;
		}
	}

	const FConstSharedStruct& SharedFragment = EntityManager.GetOrCreateConstSharedFragment(CollisionLayerFragment);
	BuildContext.AddConstSharedFragment(SharedFragment);

	BuildContext.RequireFragment<FAgentRadiusFragment>();
	BuildContext.RequireFragment<FTransformFragment>();
}
