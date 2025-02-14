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
	AddCollisionQuery.AddConstSharedRequirement<FMCCollisionLayer>();
	AddCollisionQuery.AddSubsystemRequirement<UMCWorldSubsystem>(EMassFragmentAccess::ReadWrite);
	AddCollisionQuery.RegisterWithProcessor(*this);
}

void UMCCollisionObserver::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	AddCollisionQuery.ForEachEntityChunk(EntityManager, Context, [](FMassExecutionContext& InContext)
	{
		const TConstArrayView<FAgentRadiusFragment> AgentRadiusFragments = InContext.GetFragmentView<FAgentRadiusFragment>();
		const TConstArrayView<FTransformFragment> TransformFragments = InContext.GetFragmentView<FTransformFragment>();
		const FMCCollisionLayer& CollisionLayer = InContext.GetConstSharedFragment<FMCCollisionLayer>();
		UMCWorldSubsystem* MCWorldSubsystem = InContext.GetMutableSubsystem<UMCWorldSubsystem>();
		const bool bShouldCollide = InContext.DoesArchetypeHaveTag<FMCCollidesTag>();
		
		for (int32 i = 0; i < InContext.GetNumEntities(); ++i)
		{
			float AgentRadius = AgentRadiusFragments[i].Radius;
			const FVector& EntityLocation = TransformFragments[i].GetTransform().GetLocation();
			FMassEntityHandle Entity = InContext.GetEntity(i);
			if (bShouldCollide && !MCWorldSubsystem->HasCollision(Entity))
			{
				UE::Geometry::FAxisAlignedBox3d Bounds (EntityLocation, AgentRadius);
				MCWorldSubsystem->AddCollision(Entity, Bounds, CollisionLayer.CollisionLayerIndex);
			}
			else
			{
				MCWorldSubsystem->RemoveCollision(Entity);
			}
			
		}
	});
}

//
// End UMCCollisionObserver
//


//
// Begin UMCCheckCollisionProcessor
//

FName UMCCheckCollisionProcessor::CollisionSignal = TEXT("UMCCheckCollisionProcessor_CollisionSignal");

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

		TArray<FMassEntityHandle> CollidedEntities;

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
	
			UE::Geometry::FAxisAlignedBox3d Bounds (AgentLocation, Radius);
			uint32 CellIDHint;
			if (MCWorldSubsystem.NeedsCollisionUpdate(Entity, Bounds, CellIDHint))
			{
				MCWorldSubsystem.UpdateCollision(Entity, Bounds, CellIDHint);
			}

			TArray<FMCCollision> Collisions;

			MCWorldSubsystem.RetrieveCollisions(Bounds, CollisionLayer.CollisionLayerIndex, [&](const FMassEntityHandle& OtherEntity)
			{
				if (Entity == OtherEntity || !EntityManager.IsEntityValid(OtherEntity))
				{
					return;
				}
				const FMassEntityView OtherEntityView(EntityManager, OtherEntity);
				const float OtherEntityRadius = OtherEntityView.GetFragmentData<FAgentRadiusFragment>().Radius;
				const FVector OtherEntityLocation = OtherEntityView.GetFragmentData<FTransformFragment>().GetTransform().GetLocation();
				FVector Direction = AgentLocation - OtherEntityLocation;
				if (Direction.Size2D() > OtherEntityRadius + Radius)
				{
					// No collision
					return;
				}
				Direction = Direction.GetSafeNormal2D();
				FMCCollision& Collision = Collisions.AddDefaulted_GetRef();
				Collision.HitPoint = OtherEntityLocation + Direction.GetSafeNormal2D() * OtherEntityRadius;
				Collision.OtherEntity = OtherEntity;
				Collision.Normal = Direction;
			});
	
			CollisionInformation.Collisions = Collisions;
			if (Collisions.Num() > 0)
			{
				CollidedEntities.Add(Entity);
			}
		}

		if (!CollidedEntities.IsEmpty())
		{
			SignalSubsystem.SignalEntities(CollisionSignal, CollidedEntities);
		}
	});
}

//
// End UMCCheckCollisionProcessor
//
