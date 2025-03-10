#include "Processors/MCCheckCollisionProcessor.h"

// UE Includes
#include "MassCommonFragments.h"
#include "MassEntityView.h"
#include "MassExecutionContext.h"
#include "MassMovementFragments.h"
#include "MassObserverRegistry.h"
#include "MassSignalSubsystem.h"

// MC Includes
#include "Fragments/MCMassCollisionTrait.h"
#include "Subsystems/MCWorldSubsystem.h"


//
// Begin UMCCollisionObserver
//

UMCCollisionObserver::UMCCollisionObserver()
{
	ExecutionFlags = static_cast<int32>(EProcessorExecutionFlags::AllNetModes);
	ObservedType = FAgentRadiusFragment::StaticStruct();
	Operation = EMassObservedOperation::Add;
	ExecutionOrder.ExecuteBefore.Add(UMCCheckCollisionProcessor::CollisionGroup);
}

void UMCCollisionObserver::Register()
{
	UMassObserverProcessor::Register();

	UMassObserverRegistry& ObserverRegistry = UMassObserverRegistry::GetMutable();
	ObserverRegistry.RegisterObserver(*FAgentRadiusFragment::StaticStruct(), EMassObservedOperation::Remove, GetClass());
	ObserverRegistry.RegisterObserver(*FMCCollidesTag::StaticStruct(), EMassObservedOperation::Remove, GetClass());
}

void UMCCollisionObserver::ConfigureQueries()
{
	AddCollisionQuery.AddRequirement<FAgentRadiusFragment>(EMassFragmentAccess::ReadOnly);
	AddCollisionQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	AddCollisionQuery.AddTagRequirement<FMCCollidesTag>(EMassFragmentPresence::Optional);
	AddCollisionQuery.AddRequirement<FMCCollisionsInformation>(EMassFragmentAccess::ReadWrite);
	AddCollisionQuery.AddConstSharedRequirement<FMCCollisionLayer>();
	AddCollisionQuery.AddSubsystemRequirement<UMCWorldSubsystem>(EMassFragmentAccess::ReadWrite);
	AddCollisionQuery.AddSubsystemRequirement<UMassSignalSubsystem>(EMassFragmentAccess::ReadWrite);
	AddCollisionQuery.RegisterWithProcessor(*this);
}

void UMCCollisionObserver::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	AddCollisionQuery.ForEachEntityChunk(EntityManager, Context, [](FMassExecutionContext& InContext)
	{
		const TConstArrayView<FAgentRadiusFragment> AgentRadiusFragments = InContext.GetFragmentView<FAgentRadiusFragment>();
		const TConstArrayView<FTransformFragment> TransformFragments = InContext.GetFragmentView<FTransformFragment>();
		const TArrayView<FMCCollisionsInformation> CollisionFragments = InContext.GetMutableFragmentView<FMCCollisionsInformation>();
		const FMCCollisionLayer& CollisionLayer = InContext.GetConstSharedFragment<FMCCollisionLayer>();
		UMCWorldSubsystem* MCWorldSubsystem = InContext.GetMutableSubsystem<UMCWorldSubsystem>();
		const bool bShouldCollide = InContext.DoesArchetypeHaveTag<FMCCollidesTag>();

		TArray<FMassEntityHandle> EntitiesWithOldCollisions;
		
		for (int32 i = 0; i < InContext.GetNumEntities(); ++i)
		{
			float AgentRadius = AgentRadiusFragments[i].Radius;
			const FVector& EntityLocation = TransformFragments[i].GetTransform().GetLocation();
			FMassEntityHandle Entity = InContext.GetEntity(i);
			if (bShouldCollide && !MCWorldSubsystem->HasCollision(Entity))
			{
				FSphere Sphere (EntityLocation, AgentRadius);
				MCWorldSubsystem->AddCollision(Entity, Sphere, CollisionLayer.CollisionLayerIndex);
			}
			else
			{
				FMCCollisionsInformation& CollisionInfo = CollisionFragments[i];
				CollisionInfo.PreviousCollisions = CollisionInfo.Collisions;
				CollisionInfo.NewCollisions.Empty();
				CollisionInfo.Collisions.Empty();
				if (!CollisionInfo.PreviousCollisions.IsEmpty())
				{
					EntitiesWithOldCollisions.Add(Entity);
				}
				
				MCWorldSubsystem->RemoveCollision(Entity);
			}
		}

		if (!EntitiesWithOldCollisions.IsEmpty())
		{
			UMassSignalSubsystem& SignalSubsystem = InContext.GetMutableSubsystemChecked<UMassSignalSubsystem>();
			SignalSubsystem.SignalEntities(UMCCheckCollisionProcessor::CollisionStopSignal, EntitiesWithOldCollisions);
		}
	});
}

//
// End UMCCollisionObserver
//


//
// Begin UMCCheckCollisionProcessor
//

FName UMCCheckCollisionProcessor::CollisionStartSignal = TEXT("UMCCheckCollisionProcessor_CollisionStartSignal");
FName UMCCheckCollisionProcessor::CollisionStopSignal = TEXT("UMCCheckCollisionProcessor_CollisionStopSignal");

FName UMCCheckCollisionProcessor::CollisionGroup = TEXT("UMCCheckCollisionProcessor_CollisionGroup");

UMCCheckCollisionProcessor::UMCCheckCollisionProcessor()
{
	ExecutionFlags = static_cast<int32>(EProcessorExecutionFlags::AllNetModes);
	bAutoRegisterWithProcessingPhases = true;
	ExecutionOrder.ExecuteInGroup = CollisionGroup;
	ExecutionOrder.ExecuteBefore.Add(UE::Mass::ProcessorGroupNames::Movement);
}

void UMCCheckCollisionProcessor::ConfigureQueries()
{
	EntityQuery.AddRequirement<FAgentRadiusFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FMCCollisionsInformation>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddTagRequirement<FMCCollidesTag>(EMassFragmentPresence::All);
	EntityQuery.AddConstSharedRequirement<FMCCollisionLayer>();
	EntityQuery.AddSubsystemRequirement<UMCWorldSubsystem>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddSubsystemRequirement<UMassSignalSubsystem>(EMassFragmentAccess::ReadWrite);
	EntityQuery.RegisterWithProcessor(*this);
}

void UMCCheckCollisionProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	EntityQuery.ForEachEntityChunk(EntityManager, Context, [&EntityManager](FMassExecutionContext& Context)
	{
		const TConstArrayView<FAgentRadiusFragment> AgentRadiusFragments = Context.GetFragmentView<FAgentRadiusFragment>();
		const TConstArrayView<FTransformFragment> TransformFragments = Context.GetFragmentView<FTransformFragment>();
		const TArrayView<FMCCollisionsInformation> CollisionFragments = Context.GetMutableFragmentView<FMCCollisionsInformation>();
		const FMCCollisionLayer& CollisionLayer = Context.GetConstSharedFragment<FMCCollisionLayer>();
		
		UMCWorldSubsystem& MCWorldSubsystem = Context.GetMutableSubsystemChecked<UMCWorldSubsystem>();
		UMassSignalSubsystem& SignalSubsystem = Context.GetMutableSubsystemChecked<UMassSignalSubsystem>();

		TArray<FMassEntityHandle> EntitiesWithNewCollisions;
		TArray<FMassEntityHandle> EntitiesWithRemovedCollisions;

		for (int32 i = 0; i < Context.GetNumEntities(); ++i)
		{
			const float Radius = AgentRadiusFragments[i].Radius;
			const FVector& AgentLocation = TransformFragments[i].GetTransform().GetLocation();
			FMCCollisionsInformation& CollisionInformation = CollisionFragments[i];
			
			FMassEntityHandle Entity = Context.GetEntity(i);

			if (!MCWorldSubsystem.HasCollision(Entity))
			{
				continue;
			}
			
			FSphere Bounds (AgentLocation, Radius);
			MCWorldSubsystem.UpdateCollision(Entity, Bounds);

			TArray<FMCCollision> CurrentCollisions;
			MCWorldSubsystem.RetrieveCollisions(Bounds, CollisionLayer.CollisionLayerIndex, [&](const FMassEntityHandle& OtherEntity)
			{
				if (Entity == OtherEntity || !EntityManager.IsEntityValid(OtherEntity))
				{
					return;
				}
				const FMassEntityView OtherEntityView(EntityManager, OtherEntity);
				const float OtherEntityRadius = OtherEntityView.GetFragmentData<FAgentRadiusFragment>().Radius;
				const FVector OtherEntityLocation = OtherEntityView.GetFragmentData<FTransformFragment>().GetTransform().GetLocation();
				const FVector Direction = (AgentLocation - OtherEntityLocation).GetSafeNormal2D();
				FMCCollision& Collision = CurrentCollisions.AddDefaulted_GetRef();
				Collision.HitPoint = OtherEntityLocation + Direction * OtherEntityRadius;
				Collision.OtherEntity = OtherEntity;
				Collision.Normal = Direction;
			});

			TArray<FMCCollision> NewCollisions;
			CollisionInformation.PreviousCollisions.Reset();
			CollisionInformation.NewCollisions = CurrentCollisions;
			
			for (int32 OldCollisionIndex = CollisionInformation.Collisions.Num() - 1; OldCollisionIndex >= 0; --OldCollisionIndex)
			{
				const FMCCollision& OldCollision = CollisionInformation.Collisions[OldCollisionIndex];
				bool bCollisionStopped = true;
				for (int32 NewCollisionIndex = CollisionInformation.NewCollisions.Num() - 1; NewCollisionIndex >= 0; --NewCollisionIndex)
				{
					FMCCollision& NewCollision = CollisionInformation.NewCollisions[NewCollisionIndex];
					if (OldCollision.OtherEntity == NewCollision.OtherEntity)
					{
						bCollisionStopped = false;
						CollisionInformation.NewCollisions.RemoveAt(NewCollisionIndex);
						break;
					}
				}
				if (bCollisionStopped)
				{
					CollisionInformation.PreviousCollisions.Add(OldCollision);
				}
			}
			
			CollisionInformation.Collisions = CurrentCollisions;
	
			if (CollisionInformation.NewCollisions.Num() > 0)
			{
				EntitiesWithNewCollisions.Add(Entity);
			}
			if (CollisionInformation.PreviousCollisions.Num() > 0)
			{
				EntitiesWithRemovedCollisions.Add(Entity);
			}
		}

		if (!EntitiesWithNewCollisions.IsEmpty())
		{
			SignalSubsystem.SignalEntities(CollisionStartSignal, EntitiesWithNewCollisions);
		}
		if (!EntitiesWithRemovedCollisions.IsEmpty())
		{
			SignalSubsystem.SignalEntities(CollisionStopSignal, EntitiesWithRemovedCollisions);
		}
	});
}

//
// End UMCCheckCollisionProcessor
//
