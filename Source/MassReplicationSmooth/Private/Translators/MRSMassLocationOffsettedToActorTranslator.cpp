#include "Translators/MRSMassLocationOffsettedToActorTranslator.h"

// UE Includes
#include "MassCommonTypes.h"
#include "MassExecutionContext.h"
#include "Translators/MassCharacterMovementTranslators.h"
#include "Translators/MassSceneComponentLocationTranslator.h"

// MRS Includes
#include "Fragments/MRSMeshTranslationOffset.h"

UMRSMassLocationOffsettedTranslatorBase::UMRSMassLocationOffsettedTranslatorBase()
{
	ExecutionFlags = static_cast<int32>(EProcessorExecutionFlags::All);
}

void UMRSMassLocationOffsettedTranslatorBase::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.Initialize(EntityManager);
	AddRequiredTagsToQuery(EntityQuery);
	EntityQuery.AddRequirement<FMassSceneComponentWrapperFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FMRSMeshTranslationOffset>(EMassFragmentAccess::ReadOnly);
	EntityQuery.RegisterWithProcessor(*this);
}

UMRSMassLocationOffsettedToActorTranslator::UMRSMassLocationOffsettedToActorTranslator()
{
	bRequiresGameThreadExecution = true;
	RequiredTags.Add<FMRSTranslateMassToLocationTag>();
}

void UMRSMassLocationOffsettedToActorTranslator::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	Super::ConfigureQueries(EntityManager);
	
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddTagRequirement<FMassCharacterMovementCopyToMassTag>(EMassFragmentPresence::None);
}

void UMRSMassLocationOffsettedToActorTranslator::Execute(FMassEntityManager& EntityManager,
	FMassExecutionContext& Context)
{
	EntityQuery.ForEachEntityChunk(EntityManager, Context, [this](FMassExecutionContext& Context)
	{
		const TConstArrayView<FMassSceneComponentWrapperFragment> ComponentList = Context.GetFragmentView<FMassSceneComponentWrapperFragment>();
		const TConstArrayView<FTransformFragment> LocationList = Context.GetFragmentView<FTransformFragment>();
		const TConstArrayView<FMRSMeshTranslationOffset> OffsetList = Context.GetFragmentView<FMRSMeshTranslationOffset>();


		const int32 NumEntities = Context.GetNumEntities();
		for (int32 i = 0; i < NumEntities; ++i)
		{
			if (USceneComponent* AsComponent = ComponentList[i].Component.Get())
			{
				FVector Offset = FVector(0.f, 0.f, AsComponent->Bounds.BoxExtent.Z);
				Offset += OffsetList[i].TranslationOffset;
				AsComponent->SetWorldLocation(LocationList[i].GetTransform().GetLocation() + Offset);
			}
		}
	});
}

UMRSActorToMassLocationOffsettedTranslator::UMRSActorToMassLocationOffsettedTranslator()
{
	RequiredTags.Add<FMRSTranslateLocationToMassTag>();
}

void UMRSActorToMassLocationOffsettedTranslator::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	Super::ConfigureQueries(EntityManager);
	
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddTagRequirement<FMassCharacterMovementCopyToMassTag>(EMassFragmentPresence::All);
}

void UMRSActorToMassLocationOffsettedTranslator::Execute(FMassEntityManager& EntityManager,
	FMassExecutionContext& Context)
{
	EntityQuery.ForEachEntityChunk(EntityManager, Context, [this](FMassExecutionContext& Context)
{
	const TConstArrayView<FMassSceneComponentWrapperFragment> ComponentList = Context.GetFragmentView<FMassSceneComponentWrapperFragment>();
	const TArrayView<FTransformFragment> LocationList = Context.GetMutableFragmentView<FTransformFragment>();
	const TConstArrayView<FMRSMeshTranslationOffset> OffsetList = Context.GetFragmentView<FMRSMeshTranslationOffset>();

	const int32 NumEntities = Context.GetNumEntities();
	for (int32 i = 0; i < NumEntities; ++i)
	{
		if (USceneComponent* AsComponent = ComponentList[i].Component.Get())
		{
			FVector Offset = FVector(0.f, 0.f, AsComponent->Bounds.BoxExtent.Z);
			Offset += OffsetList[i].TranslationOffset;
			LocationList[i].GetMutableTransform().SetLocation(AsComponent->GetComponentLocation() - Offset);
		}
	}
});
}

