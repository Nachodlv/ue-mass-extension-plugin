#pragma once

#include "CoreMinimal.h"
#include "MassEntityTypes.h"

#include "MRSMeshTranslationOffset.generated.h"

/** Used to offset the static mesh representation of an entity */
USTRUCT()
struct FMRSMeshTranslationOffset : public FMassFragment
{
	GENERATED_BODY()

	UPROPERTY(Transient)
	FVector TranslationOffset = FVector::ZeroVector;
};