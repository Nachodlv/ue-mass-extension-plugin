#include "Fragments/MRSMeshTranslationOffset.h"

// UE Includes
#include "MassEntityTemplateRegistry.h"
#include "MassEntityView.h"
#include "Translators/MassSceneComponentLocationTranslator.h"

// MRS Includes
#include "Translators/MRSMassLocationOffsettedToActorTranslator.h"

FMRSMeshOffsetParams FMRSMeshOffsetParams::GetValidated() const
{
	FMRSMeshOffsetParams Params = *this;
	Params.MaxTimeToSmooth = FMath::Max(0.0f, Params.MaxTimeToSmooth);
	Params.SmoothTime = FMath::Max(0.0f, Params.SmoothTime);
	Params.MaxSmoothNetUpdateDistance = FMath::Max(0.0f, Params.MaxSmoothNetUpdateDistance);
	Params.MaxSmoothNetUpdateDistanceSqr = Params.MaxSmoothNetUpdateDistance * Params.MaxSmoothNetUpdateDistance;
	return Params;
}

void UMRSMeshOffsetTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const
{
	BuildContext.AddFragment<FMRSMeshTranslationOffset>();

	FMassEntityManager& EntityManager = UE::Mass::Utils::GetEntityManagerChecked(World);
	const FConstSharedStruct& MeshOffsetSharedFrag = EntityManager.GetOrCreateConstSharedFragment(MeshOffsetParams.GetValidated());
	BuildContext.AddConstSharedFragment(MeshOffsetSharedFrag);
}

void UMRSSyncOffsetLocationTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext,
	const UWorld& World) const
{
	BuildContext.AddFragment<FMassSceneComponentWrapperFragment>();
	BuildContext.AddFragment<FTransformFragment>();
	BuildContext.RequireFragment<FMRSMeshTranslationOffset>();

	BuildContext.GetMutableObjectFragmentInitializers().Add([=](UObject& Owner, FMassEntityView& EntityView, const EMassTranslationDirection CurrentDirection)
		{
			AActor* AsActor = Cast<AActor>(&Owner);
			if (AsActor && AsActor->GetRootComponent())
			{
				USceneComponent* Component = AsActor->GetRootComponent();
				FMassSceneComponentWrapperFragment& ComponentFragment = EntityView.GetFragmentData<FMassSceneComponentWrapperFragment>();
				ComponentFragment.Component = Component;

				FTransformFragment& TransformFragment = EntityView.GetFragmentData<FTransformFragment>();

				REDIRECT_OBJECT_TO_VLOG(Component, &Owner);
				UE_VLOG_LOCATION(&Owner, LogMass, Log, Component->GetComponentLocation(), 30, FColor::Yellow, TEXT("Initial component location"));
				UE_VLOG_LOCATION(&Owner, LogMass, Log, TransformFragment.GetTransform().GetLocation(), 30, FColor::Red, TEXT("Initial entity location"));

				// the entity is the authority
				if (CurrentDirection == EMassTranslationDirection::MassToActor)
				{
					// Temporary disabling this as it is already done earlier in the MassRepresentation and we needed to do a sweep to find the floor
					//Component->SetWorldLocation(FeetLocation, /*bSweep*/true, nullptr, ETeleportType::TeleportPhysics);
				}
				// actor is the authority
				else
				{
					TransformFragment.GetMutableTransform().SetLocation(Component->GetComponentTransform().GetLocation() - FVector(0.f, 0.f, Component->Bounds.BoxExtent.Z));
				}
			}
		});

	if (EnumHasAnyFlags(SyncDirection, EMassTranslationDirection::ActorToMass))
	{
		BuildContext.AddTranslator<UMRSActorToMassLocationOffsettedTranslator>();
	}

	if (EnumHasAnyFlags(SyncDirection, EMassTranslationDirection::MassToActor))
	{
		BuildContext.AddTranslator<UMRSMassLocationOffsettedToActorTranslator>();
	}
}
