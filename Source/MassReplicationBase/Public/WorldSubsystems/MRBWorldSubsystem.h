#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"

#include "MRBWorldSubsystem.generated.h"

UCLASS(Config = MassExtension)
class MASSREPLICATIONBASE_API UMRBWorldSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void PostInitialize() override;

private:
	/** Mass client bubbles that will be registered on the world */
	UPROPERTY(EditDefaultsOnly, Config, meta = (MetaClass = "/Script/MassReplication.MassClientBubbleInfoBase"))
	TArray<FSoftClassPath> MassClientBubblesToRegister;
};
