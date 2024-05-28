#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "MassUpdateISMProcessor.h"

#include "MRSSmoothMeshOffsetProcessor.generated.h"

/** Reduces to zero the mesh offset smoothly */
UCLASS()
class MASSREPLICATIONSMOOTH_API UMRSSmoothMeshOffsetProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:
	UMRSSmoothMeshOffsetProcessor();

	virtual void ConfigureQueries() override;

	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
	FMassEntityQuery EntityQuery;
};

/** Overrides the ISM processor to add a dynamic offset to the static mesh */
UCLASS()
class UMRSMassUpdateISMProcessor : public UMassUpdateISMProcessor
{
	GENERATED_BODY()

protected:
	virtual void ConfigureQueries() override;
	
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;
};
