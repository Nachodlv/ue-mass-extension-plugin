#include "Fragments/MRSMeshTranslationOffset.h"

#include "MassEntityTemplateRegistry.h"

FMRSMeshOffsetParams FMRSMeshOffsetParams::GetValidated() const
{
	FMRSMeshOffsetParams Params = *this;
	Params.MaxTimeToSmooth = FMath::Max(0.0f, Params.MaxTimeToSmooth);
	Params.SmoothTime = FMath::Max(0.0f, Params.SmoothTime);
	Params.MaxSmoothNetUpdateDistance = FMath::Max(0.0f, Params.MaxSmoothNetUpdateDistance);
	Params.MaxSmoothNetUpdateDistanceSqr = Params.MaxSmoothNetUpdateDistance * Params.MaxSmoothNetUpdateDistance;
	return Params;
}

void UMRSMeshOffsetTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const
{
	BuildContext.AddFragment<FMRSMeshTranslationOffset>();

	FMassEntityManager& EntityManager = UE::Mass::Utils::GetEntityManagerChecked(World);
	const FConstSharedStruct& MeshOffsetSharedFrag = EntityManager.GetOrCreateConstSharedFragment(MeshOffsetParams.GetValidated());
	BuildContext.AddConstSharedFragment(MeshOffsetSharedFrag);
}
