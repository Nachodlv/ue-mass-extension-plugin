#include "Observers/MRBMoveForwardObserver.h"

#include "MassCommonFragments.h"
#include "MassExecutionContext.h"
#include "MassMovementFragments.h"

UMRBMoveForwardObserver::UMRBMoveForwardObserver()
{
	ExecutionFlags = static_cast<int32>(EProcessorExecutionFlags::Standalone | EProcessorExecutionFlags::Server);
	ObservedType = FMassVelocityFragment::StaticStruct();
	Operation = EMassObservedOperation::Add;
	bAutoRegisterWithObserverRegistry = false;
}

void UMRBMoveForwardObserver::ConfigureQueries()
{
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FMassVelocityFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.RegisterWithProcessor(*this);
}

void UMRBMoveForwardObserver::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	EntityQuery.ForEachEntityChunk(EntityManager, Context, [ObserverSpeed = Speed](FMassExecutionContext& Context)
	{
	   const TConstArrayView<FTransformFragment> TransformFragments = Context.GetFragmentView<FTransformFragment>();
	   const TArrayView<FMassVelocityFragment> VelocityFragments = Context.GetMutableFragmentView<FMassVelocityFragment>();
	       
	   for (int32 EntityIdx = 0; EntityIdx < Context.GetNumEntities(); EntityIdx++)
	   {
		  const FTransformFragment& TransformFragment = TransformFragments[EntityIdx];
		  FMassVelocityFragment& VelocityFragment = VelocityFragments[EntityIdx];

		  VelocityFragment.Value = TransformFragment.GetTransform().Rotator().Vector().GetSafeNormal() * ObserverSpeed;
	   }
	});
}
