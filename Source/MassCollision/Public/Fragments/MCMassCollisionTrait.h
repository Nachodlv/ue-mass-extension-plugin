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

UCLASS()
class MASSCOLLISION_API UMCMassCollisionTrait : public UMassEntityTraitBase
{
	GENERATED_BODY()

public:
	virtual void BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const override;
};
