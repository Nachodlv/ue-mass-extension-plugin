#include "Processors/MRSSmoothMeshOffsetProcessor.h"

// UE Includes
#include "MassCommonFragments.h"
#include "MassCommonTypes.h"
#include "MassExecutionContext.h"
#include "MassRepresentationFragments.h"
#include "MassRepresentationSubsystem.h"
#include "MassRepresentationTypes.h"

// MRS Includes
#include "MassMovementFragments.h"
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
	EntityQuery.AddConstSharedRequirement<FMRSMeshOffsetParams>();
	EntityQuery.AddRequirement<FMassVelocityFragment>(EMassFragmentAccess::ReadOnly, EMassFragmentPresence::Optional);
	EntityQuery.RegisterWithProcessor(*this);
}

void UMRSSmoothMeshOffsetProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	EntityQuery.ForEachEntityChunk(EntityManager, Context, [](FMassExecutionContext& Context)
	{
		const TArrayView<FMRSMeshTranslationOffset>& MeshOffsetList = Context.GetMutableFragmentView<FMRSMeshTranslationOffset>();
		const TConstArrayView<FMassVelocityFragment> VelocityFragments = Context.GetFragmentView<FMassVelocityFragment>();
		const FMRSMeshOffsetParams& Params = Context.GetConstSharedFragment<FMRSMeshOffsetParams>();

		const float DeltaTime = Context.GetWorld()->DeltaTimeSeconds;
		double CurrentTimeStamps = Context.GetWorld()->GetRealTimeSeconds();

		for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); ++EntityIndex)
		{
			FMRSMeshTranslationOffset& MeshOffset = MeshOffsetList[EntityIndex];

			if (DeltaTime > Params.MaxTimeToSmooth || MeshOffset.TranslationOffset.IsNearlyZero())
			{
				MeshOffset.TranslationOffset = FVector::ZeroVector;
				continue;
			}
			
			const double TargetDelta = MeshOffset.ServerUpdateTimestamp - MeshOffset.ClientOffsetTimestamp;
			const double RemainingTime = MeshOffset.ServerUpdateTimestamp - CurrentTimeStamps;
			const double CurrentSmoothTime = TargetDelta - RemainingTime;

			float SmoothTime = Params.SmoothTime;
			if (!VelocityFragments.IsEmpty() && VelocityFragments[EntityIndex].Value.IsNearlyZero())
			{
				// If the entity is not moving then reduce the offset faster
				SmoothTime *= 0.5f;
			}
			
			if (CurrentSmoothTime > SmoothTime)
			{
				MeshOffset.TranslationOffset = FVector::ZeroVector;
				continue;
			}
			SmoothTime -= CurrentSmoothTime;
			MeshOffset.TranslationOffset *= FMath::Max(1.f - DeltaTime / SmoothTime, 0.0f);
		}
	});
}

void UMRSMassUpdateISMProcessor::ConfigureQueries()
{
	Super::ConfigureQueries();
	
	EntityQuery.AddRequirement<FMRSMeshTranslationOffset>(EMassFragmentAccess::ReadOnly, EMassFragmentPresence::Optional);
}

void UMRSMassUpdateISMProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
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
