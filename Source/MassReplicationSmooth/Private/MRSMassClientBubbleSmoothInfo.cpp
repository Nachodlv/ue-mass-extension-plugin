#include "MRSMassClientBubbleSmoothInfo.h"

#include "MassCommonFragments.h"
#include "Fragments/MRSMeshTranslationOffset.h"
#include "Net/UnrealNetwork.h"


void FMRSMassClientBubbleHandler::PostReplicatedChangeEntity(const FMassEntityView& EntityView, const FMRBReplicatedAgent& Item) const
{
	FTransformFragment& TransformFragment = EntityView.GetFragmentData<FTransformFragment>();
	const FVector PreviousLocation = TransformFragment.GetTransform().GetLocation();
	FMRBMassClientBubbleHandler::PostReplicatedChangeEntity(EntityView, Item);
	const FVector NewLocation = TransformFragment.GetTransform().GetLocation();
	

	// Offsetting the mesh to sync with the sever locations smoothly
	FMRSMeshTranslationOffset& TranslationOffset = EntityView.GetFragmentData<FMRSMeshTranslationOffset>();
	const FMRSMeshOffsetParams& OffsetParams = EntityView.GetConstSharedFragmentData<FMRSMeshOffsetParams>();
	double DistSquared = FVector::DistSquared(PreviousLocation + TranslationOffset.TranslationOffset, NewLocation);
	if (OffsetParams.MaxSmoothNetUpdateDistanceSqr > DistSquared)
	{
		TranslationOffset.TranslationOffset += PreviousLocation - NewLocation;
		TranslationOffset.ClientOffsetTimestamp = Serializer->GetWorld()->GetRealTimeSeconds();
		TranslationOffset.ServerUpdateTimestamp = Item.GetServerTimeStamp();
	}
	else
	{
		TranslationOffset.TranslationOffset = FVector::ZeroVector;
	}
}

void FMRSMassClientBubbleHandler::AddQueryRequirements(FMassEntityQuery& InQuery) const
{
	FMRBMassClientBubbleHandler::AddQueryRequirements(InQuery);

	InQuery.AddRequirement<FMRSMeshTranslationOffset>(EMassFragmentAccess::ReadWrite);
	InQuery.AddConstSharedRequirement<FMRSMeshOffsetParams>();
}

AMRSMassClientBubbleSmoothInfo::AMRSMassClientBubbleSmoothInfo(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	Serializers.Add(&SmoothSerializer);
}

void AMRSMassClientBubbleSmoothInfo::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams SharedParams;
	SharedParams.bIsPushBased = true;

	// Technically, this doesn't need to be PushModel based because it's a FastArray and they ignore it.
	DOREPLIFETIME_WITH_PARAMS_FAST(AMRSMassClientBubbleSmoothInfo, SmoothSerializer, SharedParams);
}
