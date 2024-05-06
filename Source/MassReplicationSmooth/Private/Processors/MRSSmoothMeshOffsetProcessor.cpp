#include "Processors/MRSSmoothMeshOffsetProcessor.h"

// UE Includes
#include "MassCommonFragments.h"
#include "MassCommonTypes.h"
#include "MassExecutionContext.h"
#include "MassRepresentationFragments.h"
#include "MassRepresentationSubsystem.h"
#include "MassRepresentationTypes.h"

// MRS Includes
#include "Fragments/MRSMeshTranslationOffset.h"

UMRSSmoothMeshOffsetProcessor::UMRSSmoothMeshOffsetProcessor()
{
	bAutoRegisterWithProcessingPhases = true;
	ExecutionFlags = static_cast<int32>(EProcessorExecutionFlags::Client);
	ExecutionOrder.ExecuteAfter.Add(UE::Mass::ProcessorGroupNames::Movement);
}

void UMRSSmoothMeshOffsetProcessor::ConfigureQueries()
{
	EntityQuery.AddRequirement<FMRSMeshTranslationOffset>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite, EMassFragmentPresence::Optional);
	
	EntityQuery.RegisterWithProcessor(*this);
}

void UMRSSmoothMeshOffsetProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	EntityQuery.ForEachEntityChunk(EntityManager, Context, [](FMassExecutionContext& Context)
{
	const TArrayView<FMRSMeshTranslationOffset>& MeshOffsetList = Context.GetMutableFragmentView<FMRSMeshTranslationOffset>();

	const float DeltaTime = Context.GetWorld()->DeltaTimeSeconds;

	for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); ++EntityIndex)
	{
		FMRSMeshTranslationOffset& MeshOffset = MeshOffsetList[EntityIndex];
			
		constexpr float MaxDeltaTimeToSmooth = 1.0f;
		if (DeltaTime < MaxDeltaTimeToSmooth)
		{
			constexpr float SmoothTime = 0.2f;
			MeshOffset.TranslationOffset *= (1.0f - DeltaTime / SmoothTime);
		}
		else
		{
			MeshOffset.TranslationOffset = FVector::ZeroVector;
		}
	}
});
}

void UUSMassUpdateISMProcessor::ConfigureQueries()
{
	Super::ConfigureQueries();
	
	EntityQuery.AddRequirement<FMRSMeshTranslationOffset>(EMassFragmentAccess::ReadOnly, EMassFragmentPresence::Optional);
}

void UUSMassUpdateISMProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
		EntityQuery.ForEachEntityChunk(EntityManager, Context, [](FMassExecutionContext& Context)
	{
		UMassRepresentationSubsystem* RepresentationSubsystem = Context.GetSharedFragment<FMassRepresentationSubsystemSharedFragment>().RepresentationSubsystem;
		check(RepresentationSubsystem);
		FMassInstancedStaticMeshInfoArrayView ISMInfo = RepresentationSubsystem->GetMutableInstancedStaticMeshInfos();

		const TConstArrayView<FTransformFragment> TransformList = Context.GetFragmentView<FTransformFragment>();
		const TArrayView<FMassRepresentationFragment> RepresentationList = Context.GetMutableFragmentView<FMassRepresentationFragment>();
		const TConstArrayView<FMassRepresentationLODFragment> RepresentationLODList = Context.GetFragmentView<FMassRepresentationLODFragment>();
		const TConstArrayView<FMRSMeshTranslationOffset> MeshOffsetList = Context.GetFragmentView<FMRSMeshTranslationOffset>();

		for (int32 EntityIdx = 0; EntityIdx < Context.GetNumEntities(); EntityIdx++)
		{
			const FTransformFragment& TransformFragment = TransformList[EntityIdx];
			const FMassRepresentationLODFragment& RepresentationLOD = RepresentationLODList[EntityIdx];
			FMassRepresentationFragment& Representation = RepresentationList[EntityIdx];

			FVector MeshTranslationOffset = FVector::ZeroVector;
			if (MeshOffsetList.Num() > 0)
			{
				MeshTranslationOffset = MeshOffsetList[EntityIdx].TranslationOffset;
			}

			FTransform Transform = TransformFragment.GetTransform();
			if (Representation.CurrentRepresentation == EMassRepresentationType::StaticMeshInstance)
			{
				Transform.SetLocation(Transform.GetLocation() + MeshTranslationOffset);
				UpdateISMTransform(Context.GetEntity(EntityIdx), ISMInfo[Representation.StaticMeshDescHandle.ToIndex()], Transform, Representation.PrevTransform, RepresentationLOD.LODSignificance, Representation.PrevLODSignificance);
			}
			Representation.PrevTransform = Transform;
			Representation.PrevLODSignificance = RepresentationLOD.LODSignificance;
		}
	});
}
