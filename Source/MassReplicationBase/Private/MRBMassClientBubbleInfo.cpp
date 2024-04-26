﻿#include "MRBMassClientBubbleInfo.h"

// UE Includes
#include "Net/UnrealNetwork.h"
#include "MassCommonFragments.h"

// MRB Includes
#include "..\Public\MRBMassFastArray.h"

#if UE_REPLICATION_COMPILE_SERVER_CODE

FMRBMassFastArrayItem* FMRBMassClientBubbleHandler::GetMutableItem(FMassReplicatedAgentHandle Handle)
{
	if (AgentHandleManager.IsValidHandle(Handle))
	{
		const FMassAgentLookupData& LookUpData = AgentLookupArray[Handle.GetIndex()];

		return &(*Agents)[LookUpData.AgentsIdx];
	}
	return nullptr;
}

void FMRBMassClientBubbleHandler::MarkItemDirty(FMRBMassFastArrayItem& Item) const
{
	Serializer->MarkItemDirty(Item);
}

#endif // UE_REPLICATION_COMPILE_SERVER_CODE

#if UE_REPLICATION_COMPILE_CLIENT_CODE

void FMRBMassClientBubbleHandler::PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize)
{
	TArrayView<FTransformFragment> TransformFragments;

	// Add the requirements for the query used to grab all the transform fragments
	auto AddRequirementsForSpawnQuery = [this](FMassEntityQuery& InQuery)
	{
		InQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
	};

	// Cache the transform fragments
	auto CacheFragmentViewsForSpawnQuery = [&]
		(FMassExecutionContext& InExecContext)
	{
		TransformFragments = InExecContext.GetMutableFragmentView<FTransformFragment>();
	};

	// Called when a new entity is spawned. Stores the entity location in the transform fragment
	auto SetSpawnedEntityData = [&]
		(const FMassEntityView& EntityView, const FMRBReplicatedAgent& ReplicatedEntity, const int32 EntityIdx)
	{
		TransformFragments[EntityIdx].GetMutableTransform().SetLocation(ReplicatedEntity.GetEntityLocation());
	};

	// PostReplicatedChangeEntity is called when there are multiples adds without a remove so it's treated as a change
	PostReplicatedAddHelper(AddedIndices, AddRequirementsForSpawnQuery, CacheFragmentViewsForSpawnQuery, SetSpawnedEntityData, PostReplicatedChangeEntity);
}

void FMRBMassClientBubbleHandler::PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize)
{
	PostReplicatedChangeHelper(ChangedIndices, PostReplicatedChangeEntity);
}

void FMRBMassClientBubbleHandler::PostReplicatedChangeEntity(const FMassEntityView& EntityView, const FMRBReplicatedAgent& Item)
{
	// Grabs the transform fragment from the entity
	FTransformFragment& TransformFragment = EntityView.GetFragmentData<FTransformFragment>();

	// Sets the transform location with the agent location
	TransformFragment.GetMutableTransform().SetLocation(Item.GetEntityLocation());
}

#endif //UE_REPLICATION_COMPILE_CLIENT_CODE

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
