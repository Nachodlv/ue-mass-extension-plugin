#pragma once

#include "CoreMinimal.h"
#include "MassReplicationBase/Public/MRBMassClientBubbleInfo.h"
#include "Net/Serialization/FastArraySerializer.h"

#include "MRSMassClientBubbleSmoothInfo.generated.h"

/** Inserts the data that the server replicated into the fragments */
class FMRSMassClientBubbleHandler : public FMRBMassClientBubbleHandler
{
public:

protected:
#if UE_REPLICATION_COMPILE_CLIENT_CODE
	virtual void PostReplicatedChangeEntity(const FMassEntityView& EntityView, const FMRBReplicatedAgent& Item) const override;
#endif //UE_REPLICATION_COMPILE_CLIENT_CODE
};

/** Mass client bubble, there will be one of these per client and it will handle replicating the fast array of Agents between the server and clients */
USTRUCT()
struct FMRSMassClientBubbleSerializer : public FMRBMassClientBubbleSerializer
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

	/** The one responsible of storing the server data in the client fragments */
	FMRBMassClientBubbleHandler Bubble;
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


UCLASS()
class MASSREPLICATIONSMOOTH_API AMRSMassClientBubbleSmoothInfo : public AMRBMassClientBubbleInfo
{
	GENERATED_BODY()


};
