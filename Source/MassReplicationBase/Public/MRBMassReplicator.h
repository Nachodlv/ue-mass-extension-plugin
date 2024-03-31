#pragma once

// UE Includes
#include "CoreMinimal.h"
#include "MassReplicationProcessor.h"

#include "MRBMassReplicator.generated.h"

/** Stores the fragment data into UObjects to it can be replicated */
UCLASS()
class UMRBMassReplicator : public UMassReplicatorBase
{
	GENERATED_BODY()

public:
	/** Adds the replicated fragments to the query as requirements */
	virtual void AddRequirements(FMassEntityQuery& EntityQuery) override;
	
	virtual void ProcessClientReplication(FMassExecutionContext& Context, FMassReplicationContext& ReplicationContext) override;
};

