#pragma once

#include "CoreMinimal.h"
#include "MassTranslator.h"

#include "MRSMassLocationOffsettedToActorTranslator.generated.h"

UCLASS(Abstract)
class MASSREPLICATIONSMOOTH_API UMRSMassLocationOffsettedTranslatorBase : public UMassTranslator
{
	GENERATED_BODY()

public:
	UMRSMassLocationOffsettedTranslatorBase();

	virtual void ConfigureQueries() override;

protected:
	FMassEntityQuery EntityQuery;
};

UCLASS()
class MASSREPLICATIONSMOOTH_API UMRSMassLocationOffsettedToActorTranslator : public UMRSMassLocationOffsettedTranslatorBase
{
	GENERATED_BODY()

public:
	UMRSMassLocationOffsettedToActorTranslator();
	
	virtual void ConfigureQueries() override;

	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;
};

UCLASS()
class MASSREPLICATIONSMOOTH_API UMRSActorToMassLocationOffsettedTranslator : public UMRSMassLocationOffsettedTranslatorBase
{
	GENERATED_BODY()

public:
	virtual void ConfigureQueries() override;

	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;
};