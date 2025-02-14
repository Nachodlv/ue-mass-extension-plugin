#include "MRBMassReplicator.h"

// MRB Includes
#include "MRBMassClientBubbleInfo.h"
#include "MRBMassFastArray.h"
#include "MassReplicationTransformHandlers.h"
#include "Handlers/MRBMassReplicationHandlers.h"


void UMRBMassReplicator::AddRequirements(FMassEntityQuery& EntityQuery)
{
	FMassReplicationProcessorTransformHandlerBase::AddRequirements(EntityQuery);
}

void UMRBMassReplicator::ProcessClientReplication(FMassExecutionContext& Context, FMassReplicationContext& ReplicationContext)
{
#if UE_REPLICATION_COMPILE_SERVER_CODE
	
	// Cached variables used in the other lambda functions
	FMassReplicationSharedFragment* RepSharedFrag = nullptr;
	
	FMRBMassReplicationTransform TransformHandlerBase;

	auto CacheViewsCallback = [&] (FMassExecutionContext& InContext)
	{
		TransformHandlerBase.CacheFragmentViews(InContext);
		RepSharedFrag = &InContext.GetMutableSharedFragment<FMassReplicationSharedFragment>();
	};

	auto AddEntityCallback = [&] (FMassExecutionContext& InContext, const int32 EntityIdx, FMRBReplicatedAgent& InReplicatedAgent, const FMassClientHandle ClientHandle)
	{
		// Retrieves the bubble of the relevant client
		AMRBMassClientBubbleInfo& BubbleInfo = RepSharedFrag->GetTypedClientBubbleInfoChecked<AMRBMassClientBubbleInfo>(ClientHandle);

		// Sets the location in the entity agent
		TransformHandlerBase.AddEntity(EntityIdx, InReplicatedAgent.GetReplicatedPositionWithTimestamp());

		// Adds the new agent in the client bubble
		TMRBMassClientBubbleHandler* ClientHandler = static_cast<TMRBMassClientBubbleHandler*>(BubbleInfo.GetBubbleSerializer().GetClientHandler());
		return ClientHandler->AddAgent(InContext.GetEntity(EntityIdx), InReplicatedAgent);
	};

	auto ModifyEntityCallback = [&]
		(FMassExecutionContext& InContext, const int32 EntityIdx, const EMassLOD::Type LOD, const double Time, const FMassReplicatedAgentHandle Handle,
			const FMassClientHandle ClientHandle)
	{
		// Grabs the client bubble
		AMRBMassClientBubbleInfo& BubbleInfo = RepSharedFrag->GetTypedClientBubbleInfoChecked<AMRBMassClientBubbleInfo>(ClientHandle);
		TMRBMassClientBubbleHandler* Bubble = static_cast<TMRBMassClientBubbleHandler*>(BubbleInfo.GetBubbleSerializer().GetClientHandler());

		TransformHandlerBase.ModifyEntity(Handle, EntityIdx, Time, Bubble->GetLocationHandler());
	};

	auto RemoveEntityCallback = [&](FMassExecutionContext& Context, const FMassReplicatedAgentHandle Handle, const FMassClientHandle ClientHandle)
	{
		// Retrieve the client bubble
		AMRBMassClientBubbleInfo& BubbleInfo = RepSharedFrag->GetTypedClientBubbleInfoChecked<AMRBMassClientBubbleInfo>(ClientHandle);
		TMRBMassClientBubbleHandler* Bubble = static_cast<TMRBMassClientBubbleHandler*>(BubbleInfo.GetBubbleSerializer().GetClientHandler());

		// Remove the entity agent from the bubble
		Bubble->RemoveAgent(Handle);
	};
 
	CalculateClientReplication<FMRBMassFastArrayItem>(Context, ReplicationContext, CacheViewsCallback, AddEntityCallback, ModifyEntityCallback, RemoveEntityCallback);
#endif // UE_REPLICATION_COMPILE_SERVER_CODE
}
