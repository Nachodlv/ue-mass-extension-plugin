#pragma once

#include "MassReplicationTypes.h"
#include "MassClientBubbleHandler.h"

#include "MRBMassFastArray.generated.h"

/** The data that is replicated specific to each entity */
USTRUCT()
struct FMRBReplicatedAgent : public FReplicatedAgentBase
{
	GENERATED_BODY()
	
	const FVector& GetEntityLocation() const { return EntityLocation; }
	void SetEntityLocation(const FVector& InEntityLocation) { EntityLocation = InEntityLocation; }

	void SetServerTimeStamp(float TimeStamp) { ServerTimeStamps = TimeStamp; }
	float GetServerTimeStamp() const { return ServerTimeStamps; }

private:
	UPROPERTY(Transient)
	FVector_NetQuantize EntityLocation;

	/** Time on server when the agent was updated */
	UPROPERTY(Transient)
	float ServerTimeStamps = 0;
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