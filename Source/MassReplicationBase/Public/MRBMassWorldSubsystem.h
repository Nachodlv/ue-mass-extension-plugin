#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "MRBMassWorldSubsystem.generated.h"


UCLASS()
class UMRBMassWorldSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	
	virtual void PostInitialize() override;
};
