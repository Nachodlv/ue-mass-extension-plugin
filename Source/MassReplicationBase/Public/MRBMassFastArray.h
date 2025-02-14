#pragma once

#include "MassReplicationTypes.h"

#include "MRBMassFastArray.generated.h"

USTRUCT()
struct FMRBReplicatedPositionWithTimestamp
{
	GENERATED_BODY()

	UPROPERTY(Transient)
	FVector_NetQuantize Position;

	UPROPERTY(Transient)
	float Timestamp = 0.0f;
};

/** The data that is replicated specific to each entity */
USTRUCT()
struct FMRBReplicatedAgent : public FReplicatedAgentBase
{
	GENERATED_BODY()
	
	FMRBReplicatedPositionWithTimestamp& GetReplicatedPositionWithTimestamp() { return PositionWithTimestamp; }
	const FMRBReplicatedPositionWithTimestamp& GetReplicatedPositionWithTimestamp() const { return PositionWithTimestamp; }

private:
	FMRBReplicatedPositionWithTimestamp PositionWithTimestamp;
};

/** Fast array item for efficient agent replication. Remember to make this dirty if any FReplicatedCrowdAgent member variables are modified */
USTRUCT()
struct MASSREPLICATIONBASE_API FMRBMassFastArrayItem : public FMassFastArrayItemBase
{
	GENERATED_BODY()

	FMRBMassFastArrayItem() = default;
	FMRBMassFastArrayItem(const FMRBReplicatedAgent& InAgent, const FMassReplicatedAgentHandle InHandle)
		: FMassFastArrayItemBase(InHandle)
		, Agent(InAgent)
	{}

	/** This typedef is required to be provided in FMassFastArrayItemBase derived classes (with the associated FReplicatedAgentBase derived class) */
	typedef FMRBReplicatedAgent FReplicatedAgentType;

	UPROPERTY()
	FMRBReplicatedAgent Agent;
};