#pragma once

#include "CoreMinimal.h"
#include "MassClientBubbleHandler.h"
#include "MassClientBubbleInfoBase.h"
#include "MassClientBubbleSerializerBase.h"
#include "MRBMassFastArray.h"

#include "MRBMassClientBubbleInfo.generated.h"

struct FMRBMassFastArrayItem;

/** Inserts the data that the server replicated into the fragments */
class MASSREPLICATIONBASE_API FMRBMassClientBubbleHandler : public TClientBubbleHandlerBase<FMRBMassFastArrayItem>
{
public:
#if UE_REPLICATION_COMPILE_SERVER_CODE
	/** Returns the item containing the agent with given handle */
	FMRBMassFastArrayItem* GetMutableItem(FMassReplicatedAgentHandle Handle);

	/** Marks the given item as modified, so it replicates its changes to th client */
	void MarkItemDirty(FMRBMassFastArrayItem & Item) const;
#endif // UE_REPLICATION_COMPILE_SERVER_CODE

protected:
#if UE_REPLICATION_COMPILE_CLIENT_CODE
	virtual void PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize) override;
	virtual void PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize) override;

	virtual void PostReplicatedChangeEntity(const FMassEntityView& EntityView, const FMRBReplicatedAgent& Item) const;

	virtual void AddQueryRequirements(FMassEntityQuery& InQuery) const;
#endif //UE_REPLICATION_COMPILE_CLIENT_CODE

#if WITH_MASSGAMEPLAY_DEBUG && WITH_EDITOR
	virtual void DebugValidateBubbleOnServer() override {}
	virtual void DebugValidateBubbleOnClient() override {}
#endif // WITH_MASSGAMEPLAY_DEBUG && WITH_EDITOR
	
};

/** Mass client bubble, there will be one of these per client, and it will handle replicating the fast array of Agents between the server and clients */
USTRUCT()
struct FMRBMassClientBubbleSerializer : public FMassClientBubbleSerializerBase
{
	GENERATED_BODY()

public:
	FMRBMassClientBubbleSerializer()
	{
		Bubble.Initialize(Entities, *this);
	}

	/** Define a custom replication for this struct */
	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParams);

	/** The one responsible for storing the server data in the client fragments */
	FMRBMassClientBubbleHandler Bubble;

protected:
	/** Fast Array of Agents for efficient replication. Maintained as a freelist on the server, to keep index consistency as indexes are used as Handles into the Array 
	 *  Note array order is not guaranteed between server and client so handles will not be consistent between them, FMassNetworkID will be.*/
	UPROPERTY(Transient)
	TArray<FMRBMassFastArrayItem> Entities;
};

template<>
struct TStructOpsTypeTraits<FMRBMassClientBubbleSerializer> : public TStructOpsTypeTraitsBase2<FMRBMassClientBubbleSerializer>
{
	enum
	{
		// We need to use the NetDeltaSerialize function for this struct to define a custom replication
		WithNetDeltaSerializer = true,

		// Copy is not allowed for this struct
		WithCopy = false,
	};
};

/** The info actor base class that provides the actual replication */
UCLASS()
class MASSREPLICATIONBASE_API AMRBMassClientBubbleInfo : public AMassClientBubbleInfoBase
{
	GENERATED_BODY()

public:
	AMRBMassClientBubbleInfo(const FObjectInitializer& ObjectInitializer);

	virtual FMassClientBubbleSerializerBase& GetBubbleSerializer() { return BubbleSerializer; }

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	/** Contains the entities fast array */
	UPROPERTY(Replicated, Transient) 
	FMRBMassClientBubbleSerializer BubbleSerializer;
};
