#include "MRBMassReplicator.h"

// UE Includes
#include "MassCommonFragments.h"

// MRB Includes
#include "MRBMassClientBubbleInfo.h"
#include "MRBMassFastArray.h"


void UMRBMassReplicator::AddRequirements(FMassEntityQuery& EntityQuery)
{
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
}

void UMRBMassReplicator::ProcessClientReplication(FMassExecutionContext& Context,
                                                  FMassReplicationContext& ReplicationContext)
{
#if UE_REPLICATION_COMPILE_SERVER_CODE
	
	// Cached variables used in the other lambda functions
	FMassReplicationSharedFragment* RepSharedFrag = nullptr;
	TConstArrayView<FTransformFragment> TransformFragments;

	auto CacheViewsCallback = [&] (FMassExecutionContext& InContext)
	{
		TransformFragments = InContext.GetFragmentView<FTransformFragment>();
		RepSharedFrag = &InContext.GetMutableSharedFragment<FMassReplicationSharedFragment>();
	};

	auto AddEntityCallback = [&] (FMassExecutionContext& InContext, const int32 EntityIdx, FMRBReplicatedAgent& InReplicatedAgent, const FMassClientHandle ClientHandle)
	{
		// Retrieves the bubble of the relevant client
		AMRBMassClientBubbleInfo& BubbleInfo = RepSharedFrag->GetTypedClientBubbleInfoChecked<AMRBMassClientBubbleInfo>(ClientHandle);

		// Sets the location in the entity agent
		InReplicatedAgent.SetEntityLocation(TransformFragments[EntityIdx].GetTransform().GetLocation());

		// Adds the new agent in the client bubble
		return BubbleInfo.GetBubbleSerializer().Bubble.AddAgent(InContext.GetEntity(EntityIdx), InReplicatedAgent);
	};

	auto ModifyEntityCallback = [&]
		(FMassExecutionContext& InContext, const int32 EntityIdx, const EMassLOD::Type LOD, const double Time, const FMassReplicatedAgentHandle Handle,
			const FMassClientHandle ClientHandle)
	{
		// Grabs the client bubble
		AMRBMassClientBubbleInfo& BubbleInfo = RepSharedFrag->GetTypedClientBubbleInfoChecked<AMRBMassClientBubbleInfo>(ClientHandle);
		FMRBMassClientBubbleHandler& Bubble = BubbleInfo.GetBubbleSerializer().Bubble;

		// Retrieves the entity agent
		FMRBMassFastArrayItem* Item = Bubble.GetMutableItem(Handle);

		bool bMarkItemDirty = false;

		const FVector& EntityLocation = TransformFragments[EntityIdx].GetTransform().GetLocation();
		constexpr float LocationTolerance = 10.0f;
		if (!FVector::PointsAreNear(EntityLocation, Item->Agent.GetEntityLocation(), LocationTolerance))
		{
			// Only updates the agent position if the transform fragment location has changed
			Item->Agent.SetEntityLocation(EntityLocation);
			bMarkItemDirty = true;
		}

		if (bMarkItemDirty)
		{
			// Marks the agent as dirty so it replicated to the client
			Bubble.MarkItemDirty(*Item);
		}
	};

	auto RemoveEntityCallback = [RepSharedFrag](FMassExecutionContext& Context, const FMassReplicatedAgentHandle Handle, const FMassClientHandle ClientHandle)
	{
		// Retrieve the client bubble
		AMRBMassClientBubbleInfo& BubbleInfo = RepSharedFrag->GetTypedClientBubbleInfoChecked<AMRBMassClientBubbleInfo>(ClientHandle);

		// Remove the entity agent from the bubble
		BubbleInfo.GetBubbleSerializer().Bubble.RemoveAgent(Handle);
	};
 
	CalculateClientReplication<FMRBMassFastArrayItem>(Context, ReplicationContext, CacheViewsCallback, AddEntityCallback, ModifyEntityCallback, RemoveEntityCallback);
#endif // UE_REPLICATION_COMPILE_SERVER_CODE
}
