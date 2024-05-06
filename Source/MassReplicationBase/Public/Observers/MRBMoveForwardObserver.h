#pragma once

#include "CoreMinimal.h"
#include "MassObserverProcessor.h"

#include "MRBMoveForwardObserver.generated.h"

/** Sets the entity velocity to its forward direction */
UCLASS()
class MASSREPLICATIONBASE_API UMRBMoveForwardObserver : public UMassObserverProcessor
{
	GENERATED_BODY()

public:
	UMRBMoveForwardObserver();

	virtual void ConfigureQueries() override;

	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
	/** How fast the entity will move in the direction of its forward facing */
	UPROPERTY(EditDefaultsOnly, Config)
	float Speed = 100.0f;
	
	FMassEntityQuery EntityQuery;
};
