#include "Translators/MRSMassLocationOffsettedToActorTranslator.h"

#include <Windows.ApplicationModel.Appointments.h>

#include "MassCommonTypes.h"
#include "MassExecutionContext.h"
#include "Fragments/MRSMeshTranslationOffset.h"
#include "Translators/MassSceneComponentLocationTranslator.h"

UMRSMassLocationOffsettedTranslatorBase::UMRSMassLocationOffsettedTranslatorBase()
{
	ExecutionFlags = static_cast<int32>(EProcessorExecutionFlags::All);
	RequiredTags.Add<FMRSTranslateLocationTag>();
}

void UMRSMassLocationOffsettedTranslatorBase::ConfigureQueries()
{
	AddRequiredTagsToQuery(EntityQuery);
	EntityQuery.AddRequirement<FMassSceneComponentWrapperFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FMRSMeshTranslationOffset>(EMassFragmentAccess::ReadOnly);
	EntityQuery.RegisterWithProcessor(*this);
}

UMRSMassLocationOffsettedToActorTranslator::UMRSMassLocationOffsettedToActorTranslator()
{
	bRequiresGameThreadExecution = true;
}

void UMRSMassLocationOffsettedToActorTranslator::ConfigureQueries()
{
	Super::ConfigureQueries();
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.RequireMutatingWorldAccess(); // due to mutating World by setting actor's location
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

void UMRSActorToMassLocationOffsettedTranslator::ConfigureQueries()
{
	Super::ConfigureQueries();
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
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

