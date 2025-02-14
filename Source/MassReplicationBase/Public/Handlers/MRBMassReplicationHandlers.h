#pragma once

#include "CoreMinimal.h"
#include "MassReplicationTransformHandlers.h"
#include "MRBMassClientBubbleHandlerBase.h"
#include "MRBMassFastArray.h"


template<typename AgentArrayItem>
class TMRBMassClientBubbleTimestampLocationHandler
{
public:
	virtual ~TMRBMassClientBubbleTimestampLocationHandler() = default;

	TMRBMassClientBubbleTimestampLocationHandler(TMRBMassClientBubbleHandlerBase<AgentArrayItem>& InOwnerHandler);
	
#if UE_REPLICATION_COMPILE_SERVER_CODE
	virtual void SetBubblePositionFromTransform(const FMassReplicatedAgentHandle Handle, const FTransform& Transform, float Time);

	static void AddRequirementsForSpawnQuery(FMassEntityQuery& InQuery);

	void CacheFragmentViewsForSpawnQuery(FMassExecutionContext& InExecContext);

	void ClearFragmentViewsForSpawnQuery();
	
	void SetSpawnedEntityData(const int32 EntityIdx, const FMRBReplicatedPositionWithTimestamp& ReplicatedPathData) const;
	
	/** Call this when an Entity that has already been spawned is modified on the client */
	virtual void SetModifiedEntityData(const FMassEntityView& EntityView, const FMRBReplicatedPositionWithTimestamp& ReplicatedPathData);
#endif // UE_REPLICATION_COMPILE_SERVER_CODE

protected:
#if UE_REPLICATION_COMPILE_CLIENT_CODE
	static void SetEntityData(FTransformFragment& TransformFragment, const FMRBReplicatedPositionWithTimestamp& ReplicatedPositionYawData);
#endif // UE_REPLICATION_COMPILE_CLIENT_CODE

	TArrayView<FTransformFragment> TransformList;
	TMRBMassClientBubbleHandlerBase<AgentArrayItem>& OwnerHandler;
};

template <typename AgentArrayItem>
TMRBMassClientBubbleTimestampLocationHandler<AgentArrayItem>::TMRBMassClientBubbleTimestampLocationHandler(
	TMRBMassClientBubbleHandlerBase<AgentArrayItem>& InOwnerHandler) : OwnerHandler(InOwnerHandler)
{
}

#if UE_REPLICATION_COMPILE_SERVER_CODE

template <typename AgentArrayItem>
void TMRBMassClientBubbleTimestampLocationHandler<AgentArrayItem>::SetBubblePositionFromTransform(
	const FMassReplicatedAgentHandle Handle, const FTransform& Transform, float Time)
{
	const int32 AgentsIdx = OwnerHandler.GetAgentLookupArray()[Handle.GetIndex()].AgentsIdx;
	bool bMarkDirty = false;

	AgentArrayItem& Item = (*OwnerHandler.GetAgents())[AgentsIdx];

	checkf(Item.Agent.GetNetID().IsValid(), TEXT("Pos should not be updated on FCrowdFastArrayItem's that have an Invalid ID! First Add the Agent!"));

	// GetReplicatedPositionYawDataMutable() must be defined in your FReplicatedAgentBase derived class
	FMRBReplicatedPositionWithTimestamp& ReplicatedPosition = Item.Agent.GetReplicatedPositionWithTimestamp();

	// Only update the Pos and mark the item as dirty if it has changed more than the tolerance
	const FVector Pos = Transform.GetLocation();
	if (!Pos.Equals(ReplicatedPosition.Position, UE::Mass::Replication::PositionReplicateTolerance))
	{
		ReplicatedPosition.Position = Pos;
		bMarkDirty = true;
	}

	if (bMarkDirty)
	{
		ReplicatedPosition.Timestamp = Time;
		OwnerHandler.GetSerializer()->MarkItemDirty(Item);
	}
}

template <typename AgentArrayItem>
void TMRBMassClientBubbleTimestampLocationHandler<AgentArrayItem>::AddRequirementsForSpawnQuery(FMassEntityQuery& InQuery)
{
	InQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
}

template <typename AgentArrayItem>
void TMRBMassClientBubbleTimestampLocationHandler<AgentArrayItem>::CacheFragmentViewsForSpawnQuery(
	FMassExecutionContext& InExecContext)
{
	TransformList = InExecContext.GetMutableFragmentView<FTransformFragment>();
}

template <typename AgentArrayItem>
void TMRBMassClientBubbleTimestampLocationHandler<AgentArrayItem>::ClearFragmentViewsForSpawnQuery()
{
	TransformList = TArrayView<FTransformFragment>();
}

template <typename AgentArrayItem>
void TMRBMassClientBubbleTimestampLocationHandler<AgentArrayItem>::SetSpawnedEntityData(const int32 EntityIdx,
	const FMRBReplicatedPositionWithTimestamp& ReplicatedPathData) const
{
	SetEntityData(TransformList[EntityIdx], ReplicatedPathData);
}

template <typename AgentArrayItem>
void TMRBMassClientBubbleTimestampLocationHandler<AgentArrayItem>::SetModifiedEntityData(const FMassEntityView& EntityView,
	const FMRBReplicatedPositionWithTimestamp& ReplicatedPathData)
{
	FTransformFragment& TransformFragment = EntityView.GetFragmentData<FTransformFragment>();
	SetEntityData(TransformFragment, ReplicatedPathData);
}

template <typename AgentArrayItem>
void TMRBMassClientBubbleTimestampLocationHandler<AgentArrayItem>::SetEntityData(FTransformFragment& TransformFragment,
	const FMRBReplicatedPositionWithTimestamp& ReplicatedPositionYawData)
{
	TransformFragment.GetMutableTransform().SetLocation(ReplicatedPositionYawData.Position);
}
#endif // UE_REPLICATION_COMPILE_SERVER_CODE



struct MASSREPLICATIONBASE_API FMRBMassReplicationTransform : public FMassReplicationProcessorTransformHandlerBase
{

public:
	void AddEntity(const int32 EntityIdx, FMRBReplicatedPositionWithTimestamp& ReplicatedPosition) const;

	template<typename AgentArrayItem>
	void ModifyEntity(const FMassReplicatedAgentHandle Handle, const int32 EntityIdx, float Time, TMRBMassClientBubbleTimestampLocationHandler<AgentArrayItem>& LocationHandler);
};

template <typename AgentArrayItem>
void FMRBMassReplicationTransform::ModifyEntity(const FMassReplicatedAgentHandle Handle,
	const int32 EntityIdx, float Time, TMRBMassClientBubbleTimestampLocationHandler<AgentArrayItem>& LocationHandler)
{
	LocationHandler.SetBubblePositionFromTransform(Handle, TransformList[EntityIdx].GetTransform(), Time);
}
