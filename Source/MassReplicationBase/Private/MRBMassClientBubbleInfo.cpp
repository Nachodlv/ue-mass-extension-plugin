﻿#include "MRBMassClientBubbleInfo.h"

// UE Includes
#include "Net/UnrealNetwork.h"
#include "MassCommonFragments.h"

// MRB Includes
#include "Net/Serialization/FastArraySerializer.h"

#if UE_REPLICATION_COMPILE_CLIENT_CODE

void TMRBMassClientBubbleHandler::PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize)
{
	// Add the requirements for the query used to grab all the transform fragments
	auto AddRequirementsForSpawnQuery = [this](FMassEntityQuery& InQuery)
	{
		LocationHandler.AddRequirementsForSpawnQuery(InQuery);
	};

	// Cache the transform fragments
	auto CacheFragmentViewsForSpawnQuery = [&]
		(FMassExecutionContext& InExecContext)
	{
		LocationHandler.CacheFragmentViewsForSpawnQuery(InExecContext);
	};

	// Called when a new entity is spawned. Stores the entity location in the transform fragment
	auto SetSpawnedEntityData = [&]
		(const FMassEntityView& EntityView, const FMRBReplicatedAgent& ReplicatedEntity, const int32 EntityIdx)
	{
		LocationHandler.SetSpawnedEntityData(EntityIdx, ReplicatedEntity.GetReplicatedPositionWithTimestamp());
	};

	auto PostReplicatedChange = [this](const FMassEntityView& EntityView, const FMRBReplicatedAgent& Item)
	{
		PostReplicatedChangeEntity(EntityView, Item);
	};

	// PostReplicatedChangeEntity is called when there are multiples adds without a remove so it's treated as a change
	PostReplicatedAddHelper(AddedIndices, AddRequirementsForSpawnQuery, CacheFragmentViewsForSpawnQuery, SetSpawnedEntityData, PostReplicatedChange);

	LocationHandler.ClearFragmentViewsForSpawnQuery();
}

void TMRBMassClientBubbleHandler::PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize)
{
	PostReplicatedChangeHelper(ChangedIndices, [this](const FMassEntityView& EntityView, const FMRBReplicatedAgent& Item)
	{
		PostReplicatedChangeEntity(EntityView, Item);
	});
}

void TMRBMassClientBubbleHandler::PostReplicatedChangeEntity(const FMassEntityView& EntityView, const FMRBReplicatedAgent& Item)
{
	LocationHandler.SetModifiedEntityData(EntityView, Item.GetReplicatedPositionWithTimestamp());
}

#endif //UE_REPLICATION_COMPILE_CLIENT_CODE

bool FMRBMassClientBubbleSerializer::NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParams)
{
	return FFastArraySerializer::FastArrayDeltaSerialize<FMRBMassFastArrayItem, FMRBMassClientBubbleSerializer>(Entities, DeltaParams, *this);
}

AMRBMassClientBubbleInfo::AMRBMassClientBubbleInfo(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	Serializers.Add(&BubbleSerializer);
}

void AMRBMassClientBubbleInfo::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams SharedParams;
	SharedParams.bIsPushBased = true;

	// Technically, this doesn't need to be PushModel based because it's a FastArray and they ignore it.
	DOREPLIFETIME_WITH_PARAMS_FAST(AMRBMassClientBubbleInfo, BubbleSerializer, SharedParams);
}
