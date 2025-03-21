#pragma once

#include "CoreMinimal.h"
#include "MassObserverProcessor.h"
#include "MassProcessor.h"
#include "UObject/Object.h"
#include "MCCheckCollisionProcessor.generated.h"

UCLASS()
class UMCCollisionObserver : public UMassObserverProcessor
{
	GENERATED_BODY()

public:
	UMCCollisionObserver();

	virtual void Register() override;

	virtual void ConfigureQueries() override;

	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;
	
private:
	FMassEntityQuery AddCollisionQuery;
};

UCLASS()
class MASSCOLLISION_API UMCCheckCollisionProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:
	UMCCheckCollisionProcessor();

	virtual void ConfigureQueries() override;

	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

	/** The signal name emitted when to entities collide with each other */
	static FName CollisionStartSignal;
	
	/** The signal name emitted when to entities stop colliding with each other */
	static FName CollisionStopSignal;

	/** The processor group name */
	static FName CollisionGroup;
	
private:
	FMassEntityQuery EntityQuery;
};
