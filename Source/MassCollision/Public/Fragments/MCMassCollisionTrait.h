#pragma once

#include "CoreMinimal.h"
#include "MassEntityTraitBase.h"
#include "MassEntityTypes.h"
#include "UObject/Object.h"

#include "MCMassCollisionTrait.generated.h"

USTRUCT()
struct FMCCollidesTag : public FMassTag
{
	GENERATED_BODY()
};

/** Contains the data for a specific collision */
struct FMCCollision
{
	/** Collision point */
	FVector HitPoint;

	FMassEntityHandle OtherEntity;

	FVector Normal;
};

USTRUCT()
struct FMCCollisionsInformation : public FMassFragment
{
	GENERATED_BODY()

	TArray<FMCCollision> Collisions;
};

USTRUCT()
struct FMCCollisionLayer : public FMassSharedFragment
{
	GENERATED_BODY()

	UPROPERTY()
	int32 CollisionLayerIndex = INDEX_NONE;
};

UCLASS()
class MASSCOLLISION_API UMCMassCollisionTrait : public UMassEntityTraitBase
{
	GENERATED_BODY()

public:
	virtual void BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const override;

	UPROPERTY(EditAnywhere, meta = (GetOptions = "MassCollision.MCCollisionLayersSettings.GetCollisionLayerNames"))
	FName CollisionLayer;
};
