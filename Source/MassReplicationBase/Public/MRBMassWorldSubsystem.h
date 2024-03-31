// Fill out your copyright notice in the Description page of Project Settings.

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
