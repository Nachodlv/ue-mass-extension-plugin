#pragma once

#include "CoreMinimal.h"
#include "Fragments/MRSMeshTranslationOffset.h"
#include "Handlers/MRBMassReplicationHandlers.h"

template<typename AgentArrayItem>
class TMRSMassClientBubbleSmoothLocationHandler : public TMRBMassClientBubbleTimestampLocationHandler<AgentArrayItem>
{
public:
	TMRSMassClientBubbleSmoothLocationHandler(TMRBMassClientBubbleHandlerBase<AgentArrayItem>& InOwnerHandler)
		: TMRBMassClientBubbleTimestampLocationHandler<AgentArrayItem>(InOwnerHandler) {}

#if UE_REPLICATION_COMPILE_SERVER_CODE
	static void AddRequirementsForSpawnQuery(FMassEntityQuery& InQuery);

	/** Call this when an Entity that has already been spawned is modified on the client */
	virtual void SetModifiedEntityData(const FMassEntityView& EntityView, const FMRBReplicatedPositionWithTimestamp& ReplicatedPathData);
#endif // UE_REPLICATION_COMPILE_SERVER_CODE

};

template <typename AgentArrayItem>
void TMRSMassClientBubbleSmoothLocationHandler<AgentArrayItem>::AddRequirementsForSpawnQuery(FMassEntityQuery& InQuery)
{
	TMRBMassClientBubbleTimestampLocationHandler<AgentArrayItem>::AddRequirementsForSpawnQuery(InQuery);
	InQuery.AddRequirement<FMRSMeshTranslationOffset>(EMassFragmentAccess::ReadWrite);
	InQuery.AddConstSharedRequirement<FMRSMeshOffsetParams>();
}

template <typename AgentArrayItem>
void TMRSMassClientBubbleSmoothLocationHandler<AgentArrayItem>::SetModifiedEntityData(const FMassEntityView& EntityView,
	const FMRBReplicatedPositionWithTimestamp& ReplicatedPathData)
{
	FTransformFragment& TransformFragment = EntityView.GetFragmentData<FTransformFragment>();
	const FVector PreviousLocation = TransformFragment.GetTransform().GetLocation();
	TMRBMassClientBubbleTimestampLocationHandler<AgentArrayItem>::SetModifiedEntityData(EntityView, ReplicatedPathData);
	const FVector NewLocation = TransformFragment.GetTransform().GetLocation();
	
	// Offsetting the mesh to sync with the sever locations smoothly
	FMRSMeshTranslationOffset& TranslationOffset = EntityView.GetFragmentData<FMRSMeshTranslationOffset>();
	const FMRSMeshOffsetParams& OffsetParams = EntityView.GetConstSharedFragmentData<FMRSMeshOffsetParams>();
	double DistSquared = FVector::DistSquared(PreviousLocation + TranslationOffset.TranslationOffset, NewLocation);
	if (OffsetParams.MaxSmoothNetUpdateDistanceSqr > DistSquared)
	{
		TranslationOffset.TranslationOffset += PreviousLocation - NewLocation;
		TranslationOffset.ClientOffsetTimestamp = this->OwnerHandler.GetSerializer()->GetWorld()->GetRealTimeSeconds();
		TranslationOffset.ServerUpdateTimestamp = ReplicatedPathData.Timestamp;
	}
	else
	{
		TranslationOffset.TranslationOffset = FVector::ZeroVector;
	}
}
