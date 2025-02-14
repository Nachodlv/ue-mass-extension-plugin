#pragma once

#include "CoreMinimal.h"
#include "Handler/MRSMassReplicationHandlers.h"
#include "MassReplicationBase/Public/MRBMassClientBubbleInfo.h"
#include "Net/Serialization/FastArraySerializer.h"

#include "MRSMassClientBubbleSmoothInfo.generated.h"

/** Inserts the data that the server replicated into the fragments */
class FMRSMassClientBubbleHandler : public TMRBMassClientBubbleHandlerBase<FMRBMassFastArrayItem>
{
public:
	FMRSMassClientBubbleHandler() : LocationHandler(*this) {}
	
#if UE_REPLICATION_COMPILE_SERVER_CODE
	TMRBMassClientBubbleTimestampLocationHandler<FMRBMassFastArrayItem>& GetLocationHandler() { return LocationHandler; }
#endif // UE_REPLICATION_COMPILE_SERVER_CODE

protected:
#if UE_REPLICATION_COMPILE_CLIENT_CODE
	virtual void PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize) override;
	virtual void PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize) override;

	virtual void PostReplicatedChangeEntity(const FMassEntityView& EntityView, const FMRBReplicatedAgent& Item);
#endif //UE_REPLICATION_COMPILE_CLIENT_CODE

#if WITH_MASSGAMEPLAY_DEBUG && WITH_EDITOR
	virtual void DebugValidateBubbleOnServer() override {}
	virtual void DebugValidateBubbleOnClient() override {}
#endif // WITH_MASSGAMEPLAY_DEBUG && WITH_EDITOR

private:
	TMRSMassClientBubbleSmoothLocationHandler<FMRBMassFastArrayItem> LocationHandler;
};

/** Mass client bubble, there will be one of these per client, and it will handle replicating the fast array of Agents between the server and clients */
USTRUCT()
struct FMRSMassClientBubbleSerializer : public FMassClientBubbleSerializerBase
{
	GENERATED_BODY()

public:
	FMRSMassClientBubbleSerializer()
	{
		Bubble.Initialize(Entities, *this);
	}

	/** Define a custom replication for this struct */
	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParams)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FMRBMassFastArrayItem, FMRSMassClientBubbleSerializer>(Entities, DeltaParams, *this);
	}

	/** The one responsible for storing the server data in the client fragments */
	FMRSMassClientBubbleHandler Bubble;

protected:
	/** Fast Array of Agents for efficient replication. Maintained as a freelist on the server, to keep index consistency as indexes are used as Handles into the Array 
	 *  Note array order is not guaranteed between server and client so handles will not be consistent between them, FMassNetworkID will be.*/
	UPROPERTY(Transient)
	TArray<FMRBMassFastArrayItem> Entities;
};

template<>
struct TStructOpsTypeTraits<FMRSMassClientBubbleSerializer> : public TStructOpsTypeTraitsBase2<FMRBMassClientBubbleSerializer>
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
class MASSREPLICATIONSMOOTH_API AMRSMassClientBubbleSmoothInfo : public AMRBMassClientBubbleInfo
{
	GENERATED_BODY()
	
public:
	AMRSMassClientBubbleSmoothInfo(const FObjectInitializer& ObjectInitializer);

	virtual FMassClientBubbleSerializerBase& GetBubbleSerializer() override { return SmoothSerializer; }

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	/** Contains the entities fast array */
	UPROPERTY(Replicated, Transient) 
	FMRSMassClientBubbleSerializer SmoothSerializer;

};
