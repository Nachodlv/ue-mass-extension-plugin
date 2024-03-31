#pragma once

#include "CoreMinimal.h"
#include "MassClientBubbleInfoBase.h"
#include "MRBMassFastArray.h"

#include "MRBMassClientBubbleInfo.generated.h"

struct FMRBMassFastArrayItem;

/** Inserts the data that the server replicated into the fragments */
class FMRBMassClientBubbleHandler : public TClientBubbleHandlerBase<FMRBMassFastArrayItem>
{
public:
	typedef TMassClientBubbleTransformHandler<FMRBMassFastArrayItem> FMassClientBubbleTransformHandler;

	
#if UE_REPLICATION_COMPILE_SERVER_CODE
	FMRBMassFastArrayItem* GetMutableItem(FMassReplicatedAgentHandle Handle);

	void MarkItemDirty(FMRBMassFastArrayItem & Item) const;
#endif // UE_REPLICATION_COMPILE_SERVER_CODE

protected:
#if UE_REPLICATION_COMPILE_CLIENT_CODE
	virtual void PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize) override;
	virtual void PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize) override;

	static void PostReplicatedChangeEntity(const FMassEntityView& EntityView, const FMRBReplicatedAgent& Item);
#endif //UE_REPLICATION_COMPILE_CLIENT_CODE

#if WITH_MASSGAMEPLAY_DEBUG && WITH_EDITOR
	virtual void DebugValidateBubbleOnServer() override {}
	virtual void DebugValidateBubbleOnClient() override {}
#endif // WITH_MASSGAMEPLAY_DEBUG && WITH_EDITOR
	
};

/** Mass client bubble, there will be one of these per client and it will handle replicating the fast array of Agents between the server and clients */
USTRUCT()
struct FMRBMassClientBubbleSerializer : public FMassClientBubbleSerializerBase
{
	GENERATED_BODY()

public:
	FMRBMassClientBubbleSerializer()
	{
		Bubble.Initialize(Entities, *this);
	}

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParams)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FMRBMassFastArrayItem, FMRBMassClientBubbleSerializer>(Entities, DeltaParams, *this);
	}

	FMRBMassClientBubbleHandler Bubble;

protected:
	/** Fast Array of Agents for efficient replication. Maintained as a freelist on the server, to keep index consistency as indexes are used as Handles into the Array 
	 *  Note array order is not guaranteed between server and client so handles will not be consistent between them, FMassNetworkID will be.
	 */
	UPROPERTY(Transient)
	TArray<FMRBMassFastArrayItem> Entities;
};

template<>
struct TStructOpsTypeTraits<FMRBMassClientBubbleSerializer> : public TStructOpsTypeTraitsBase2<FMRBMassClientBubbleSerializer>
{
	enum
	{
		WithNetDeltaSerializer = true,
		WithCopy = false,
	};
};

/** The info actor base class that provides the actual replication */
UCLASS()
class AMRBMassClientBubbleInfo : public AMassClientBubbleInfoBase
{
	GENERATED_BODY()

	
public:
	AMRBMassClientBubbleInfo(const FObjectInitializer& ObjectInitializer);

	FMRBMassClientBubbleSerializer& GetBubbleSerializer() { return BubbleSerializer; }

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	UPROPERTY(Replicated, Transient) 
	FMRBMassClientBubbleSerializer BubbleSerializer;
};
