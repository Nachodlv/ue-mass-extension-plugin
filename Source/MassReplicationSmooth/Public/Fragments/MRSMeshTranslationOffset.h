#pragma once

#include "CoreMinimal.h"
#include "MassAgentTraits.h"
#include "MassEntityTraitBase.h"
#include "MassEntityTypes.h"

#include "MRSMeshTranslationOffset.generated.h"

/** Shared params to offset the entity mesh */
USTRUCT()
struct FMRSMeshOffsetParams : public FMassSharedFragment
{
	GENERATED_BODY()

	/** Maximum time the smoothing can take. If it takes more than this values it will snap to the actual position */
	UPROPERTY(EditAnywhere, meta = (UIMin = 0.0f, ClampMin = 0.0f))
	float MaxTimeToSmooth = 0.2f;

	/** How much time the smooth can take */
	UPROPERTY(EditAnywhere, meta = (UIMin = 0.0f, ClampMin = 0.0f))
	float SmoothTime = 0.5f;

	/** The tolerated distance to smooth. If the distance is higher the mesh will snap to the actual position. */
	UPROPERTY(EditAnywhere, meta = (UIMin = 0.0f, ClampMin = 0.0f))
	float MaxSmoothNetUpdateDistance = 300.0f;

	float MaxSmoothNetUpdateDistanceSqr = 0.0f;

public:
	/** Returns a copy of this instance with the parameters validated */
	FMRSMeshOffsetParams GetValidated() const;
};

/** Used to offset the static mesh representation of an entity */
USTRUCT()
struct FMRSMeshTranslationOffset : public FMassFragment
{
	GENERATED_BODY()

	UPROPERTY(Transient)
	FVector TranslationOffset = FVector::ZeroVector;

	/** Client timestamp when the offset was initialized */
	double ClientOffsetTimestamp = 0.0;

	/** Server timestamp when the entity location changed */
	double ServerUpdateTimestamp = 0.0;
};

UCLASS()
class UMRSMeshOffsetTrait : public UMassEntityTraitBase
{
	GENERATED_BODY()

public:
	virtual void BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const override;

private:
	UPROPERTY(EditAnywhere)
	FMRSMeshOffsetParams MeshOffsetParams;
};

USTRUCT()
struct FMRSTranslateLocationTag : public FMassTag
{
	GENERATED_BODY()
};

UCLASS()
class UMRSSyncOffsetLocationTrait : public UMassAgentSyncTrait
{
	GENERATED_BODY()

public:
	virtual void BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const override;
};

