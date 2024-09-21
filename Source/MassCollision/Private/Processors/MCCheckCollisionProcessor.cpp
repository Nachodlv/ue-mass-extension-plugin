#include "Processors/MCCheckCollisionProcessor.h"

#include "MassCommonFragments.h"
#include "MassEntityView.h"
#include "MassExecutionContext.h"
#include "MassMovementFragments.h"
#include "MassObserverRegistry.h"
#include "Fragments/MCMassCollisionTrait.h"
#include "Subsystems/MCWorldSubsystem.h"

UMCCollisionObserver::UMCCollisionObserver()
{
	ExecutionFlags = static_cast<int32>(EProcessorExecutionFlags::AllNetModes);
	ObservedType = FAgentRadiusFragment::StaticStruct();
	Operation = EMassObservedOperation::Add;
}

void UMCCollisionObserver::Register()
{
	UMassObserverProcessor::Register();

	UMassObserverRegistry::GetMutable().RegisterObserver(*FAgentRadiusFragment::StaticStruct(), EMassObservedOperation::Remove, GetClass());
}

void UMCCollisionObserver::ConfigureQueries()
{
	AddCollisionQuery.AddRequirement<FAgentRadiusFragment>(EMassFragmentAccess::ReadOnly);
	AddCollisionQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	AddCollisionQuery.AddTagRequirement<FMCCollidesTag>(EMassFragmentPresence::All);
	AddCollisionQuery.AddSubsystemRequirement<UMCWorldSubsystem>(EMassFragmentAccess::ReadWrite);
	AddCollisionQuery.RegisterWithProcessor(*this);
}

void UMCCollisionObserver::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	AddCollisionQuery.ForEachEntityChunk(EntityManager, Context, [](FMassExecutionContext& InContext)
	{
		const TConstArrayView<FAgentRadiusFragment> AgentRadiusFragments = InContext.GetFragmentView<FAgentRadiusFragment>();
		const TConstArrayView<FTransformFragment> TransformFragments = InContext.GetFragmentView<FTransformFragment>();
		UMCWorldSubsystem* MCWorldSubsystem = InContext.GetMutableSubsystem<UMCWorldSubsystem>();
		
		for (int32 i = 0; i < InContext.GetNumEntities(); ++i)
		{
			float AgentRadius = AgentRadiusFragments[i].Radius;
			const FVector& EntityLocation = TransformFragments[i].GetTransform().GetLocation();
			FMassEntityHandle Entity = InContext.GetEntity(i);
			if (!MCWorldSubsystem->HasCollision(Entity))
			{
				UE::Geometry::FAxisAlignedBox3d Bounds (EntityLocation, AgentRadius);
				MCWorldSubsystem->AddCollision(Entity, Bounds);
			}
			else
			{
				MCWorldSubsystem->RemoveCollision(Entity);
			}
			
		}
	});
}

UMCCheckCollisionProcessor::UMCCheckCollisionProcessor()
{
	ExecutionFlags = static_cast<int32>(EProcessorExecutionFlags::AllNetModes);
	bAutoRegisterWithProcessingPhases = true;
	ExecutionOrder.ExecuteBefore.Add(UE::Mass::ProcessorGroupNames::Movement);
	bRequiresGameThreadExecution = true; // TODO remove
}

void UMCCheckCollisionProcessor::ConfigureQueries()
{
	EntityQuery.AddRequirement<FAgentRadiusFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FMassVelocityFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddTagRequirement<FMCCollidesTag>(EMassFragmentPresence::All);
	EntityQuery.AddSubsystemRequirement<UMCWorldSubsystem>(EMassFragmentAccess::ReadWrite);
	EntityQuery.RegisterWithProcessor(*this);
}

void UMCCheckCollisionProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	EntityQuery.ForEachEntityChunk(EntityManager, Context, [&EntityManager](FMassExecutionContext& Context)
	{
		const TConstArrayView<FAgentRadiusFragment> AgentRadiusFragments = Context.GetFragmentView<FAgentRadiusFragment>();
		const TConstArrayView<FTransformFragment> TransformFragments = Context.GetFragmentView<FTransformFragment>();
		const TArrayView<FMassVelocityFragment> velocityFragments = Context.GetMutableFragmentView<FMassVelocityFragment>();
		UMCWorldSubsystem* MCWorldSubsystem = Context.GetMutableSubsystem<UMCWorldSubsystem>();

		for (int32 i = 0; i < Context.GetNumEntities(); ++i)
		{
			const float Radius = AgentRadiusFragments[i].Radius;
			const FVector& AgentLocation = TransformFragments[i].GetTransform().GetLocation();
			FVector& Velocity = velocityFragments[i].Value;
			
			FMassEntityHandle Entity = Context.GetEntity(i);

			if (!MCWorldSubsystem->HasCollision(Entity))
			{
				continue;
			}

			UE::Geometry::FAxisAlignedBox3d Bounds (AgentLocation, Radius);
			MCWorldSubsystem->UpdateCollision(Entity, Bounds);

			MCWorldSubsystem->RetrieveCollisions(Bounds, [&](const FMassEntityHandle& OtherEntity)
			{
				if (Entity == OtherEntity)
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
				const FVector HitPoint = OtherEntityLocation + Direction.GetSafeNormal2D() * OtherEntityRadius;
				Velocity += Direction * 100.0f;
			});
		}
	});
}
