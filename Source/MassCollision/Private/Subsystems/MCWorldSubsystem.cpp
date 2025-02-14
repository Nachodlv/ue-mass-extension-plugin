#include "Subsystems/MCWorldSubsystem.h"

// UE Includes
#include "MassCommonFragments.h"
#include "MassEntitySubsystem.h"

// MC Includes
#include "Fragments/MCMassCollisionTrait.h"

#if !UE_BUILD_SHIPPING

namespace MCWorldSubsystemCVars
{
	static TAutoConsoleVariable<bool> CVarDebugCollisionShapes(
		TEXT("Mass.Collision.DebugShape"),
		false,
		TEXT("Draw the collision shapes"),
		ECVF_Default|ECVF_Cheat);
}

#endif // !UE_BUILD_SHIPPING


bool FMCCollisionLayerInfo::operator==(const FMCCollisionLayerInfo& Other) const
{
	return LayerName == Other.LayerName;
}

TOptional<int32> UMCCollisionLayersSettings::GetCollisionIndexWithName(const FName& LayerName) const
{
	for (int32 i = 0; i < CollisionLayers.Num(); i++)
	{
		if (CollisionLayers[i].LayerName == LayerName)
		{
			return i;
		}
	}
	return TOptional<int32>();
}

TArray<FName> UMCCollisionLayersSettings::GetCollisionLayerNames()
{
	const UMCCollisionLayersSettings* LayerSettings = GetDefault<UMCCollisionLayersSettings>();
	if (!LayerSettings)
	{
		return {};
	}
	
	TArray<FName> CollisionLayerNames;
	Algo::Transform(LayerSettings->CollisionLayers, CollisionLayerNames, [](const FMCCollisionLayerInfo& LayerInfo)
	{
		return LayerInfo.LayerName;
	});
	return CollisionLayerNames;
}

bool UMCWorldSubsystem::HasCollision(const FMassEntityHandle& EntityHandle)
{
	return EntityCollisionDataByMassID.Find(EntityHandle) != nullptr;
}

void UMCWorldSubsystem::AddCollision(const FMassEntityHandle& EntityHandle, const UE::Geometry::FAxisAlignedBox3d& Bounds, int32 CollisionLayerIndex)
{
	UE_MT_SCOPED_WRITE_ACCESS(OctreeAccessDetector);

	static int32 OctreeIndex = 0;
	CollisionOctree.InsertObject(OctreeIndex, Bounds);
	EntityCollisionDataByMassID.Add(EntityHandle, FMCEntityCollisionData(1 << CollisionLayerIndex, OctreeIndex));
	EntitiesByOctreeElementId.Add(OctreeIndex, EntityHandle);
	++OctreeIndex;
}

bool UMCWorldSubsystem::NeedsCollisionUpdate(const FMassEntityHandle& EntityHandle,
	const UE::Geometry::FAxisAlignedBox3d& NewBounds, uint32& CellIDOut)
{
	UE_MT_SCOPED_READ_ACCESS(OctreeAccessDetector);

	return CollisionOctree.CheckIfObjectNeedsReinsert(EntityCollisionDataByMassID[EntityHandle].OctreeIndex, NewBounds, CellIDOut);
}

void UMCWorldSubsystem::UpdateCollision(const FMassEntityHandle& EntityHandle,
	const UE::Geometry::FAxisAlignedBox3d& NewBounds, uint32 CellIDHint)
{
	UE_MT_SCOPED_WRITE_ACCESS(OctreeAccessDetector);

	CollisionOctree.ReinsertObject(EntityCollisionDataByMassID[EntityHandle].OctreeIndex, NewBounds, CellIDHint);
}

void UMCWorldSubsystem::RemoveCollision(const FMassEntityHandle& EntityHandle)
{
	UE_MT_SCOPED_WRITE_ACCESS(OctreeAccessDetector);
	
	const int32 OctreeIndex = EntityCollisionDataByMassID[EntityHandle].OctreeIndex;
	CollisionOctree.RemoveObject(OctreeIndex);
	EntityCollisionDataByMassID.Remove(EntityHandle);
	EntitiesByOctreeElementId.Remove(OctreeIndex);
}

void UMCWorldSubsystem::RetrieveCollisions(const UE::Geometry::FAxisAlignedBox3d& SearchBounds, int32 CollisionLayerIndex,
	TFunctionRef<void(const FMassEntityHandle&)> ObjectIDFunc) const
{
	UE_MT_SCOPED_READ_ACCESS(OctreeAccessDetector);

	if (!ensureMsgf(CollisionFlags.IsValidIndex(CollisionLayerIndex), TEXT("Invalid collision layer")))
	{
		return;
	}
	
	const uint8 CollisionFlag = CollisionFlags[CollisionLayerIndex];
	RetrieveCollisionsByFlag(SearchBounds, CollisionFlag, ObjectIDFunc);
}

void UMCWorldSubsystem::RetrieveCollisionsByFlag(const UE::Geometry::FAxisAlignedBox3d& SearchBounds,
	uint8 CollisionFlag, TFunctionRef<void(const FMassEntityHandle&)> ObjectIDFunc) const
{
	CollisionOctree.RangeQuery(SearchBounds, [&ObjectIDFunc, this, &CollisionFlag](int32 ObjectID)
	{
		const FMassEntityHandle& EntityHandle = EntitiesByOctreeElementId[ObjectID];
		const FMCEntityCollisionData& EntityData = EntityCollisionDataByMassID.FindChecked(EntityHandle);
		if (CollisionFlag & EntityData.CollisionLayerFlag)
		{
			ObjectIDFunc(EntityHandle);
		}
	});
}

TStatId UMCWorldSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UMCWorldSubsystem, STATGROUP_Tickables);
}

bool UMCWorldSubsystem::IsAllowedToTick() const
{
#if !UE_BUILD_SHIPPING
	return MCWorldSubsystemCVars::CVarDebugCollisionShapes.GetValueOnGameThread();
#endif // !UE_BUILD_SHIPPING
}

void UMCWorldSubsystem::Tick(float DeltaTime)
{
#if !UE_BUILD_SHIPPING
	UMassEntitySubsystem* EntitySubsystem = UWorld::GetSubsystem<UMassEntitySubsystem>(GetWorld());
	if (!EntitySubsystem)
	{
		return;
	}
	
	const FMassEntityManager& EntityManager = EntitySubsystem->GetEntityManager();
	for (const auto& CollisionData : EntityCollisionDataByMassID)
	{
		if (!EntityManager.IsEntityValid(CollisionData.Key))
		{
			continue;
		}
		
		FAgentRadiusFragment* RadiusFragment = EntityManager.GetFragmentDataPtr<FAgentRadiusFragment>(CollisionData.Key);
		FTransformFragment* TransformFragment = EntityManager.GetFragmentDataPtr<FTransformFragment>(CollisionData.Key);
		if (RadiusFragment && TransformFragment)
		{
			DrawDebugCircle(GetWorld(), TransformFragment->GetTransform().GetLocation(), RadiusFragment->Radius, 10, FColor::Red, false, -1.0f, 0, 0, FVector(1.0f, 0.0f, 0.0f), FVector(0.0f, 1.0f, 0.0f));

			if (FMCCollisionLayer* CollisionLayerFragment = EntityManager.GetConstSharedFragmentDataPtr<FMCCollisionLayer>(CollisionData.Key))
			{
				DrawDebugString(GetWorld(), TransformFragment->GetTransform().GetLocation(), FString::FromInt(CollisionLayerFragment->CollisionLayerIndex), nullptr, FColor::White, DeltaTime);
			}
		}

		if (const FMCCollisionsInformation* CollisionInformation = EntityManager.GetFragmentDataPtr<FMCCollisionsInformation>(CollisionData.Key))
		{
			for (const FMCCollision& Collision : CollisionInformation->Collisions)
			{
				DrawDebugPoint(GetWorld(), Collision.HitPoint, 10.0f, FColor::Blue, false, 3.0f);
			}
		}

	}
#endif // !UE_BUILD_SHIPPING
}

void UMCWorldSubsystem::PostInitialize()
{
	const UMCCollisionLayersSettings* LayerSettings = GetDefault<UMCCollisionLayersSettings>();
	if (!ensure(LayerSettings) && ensure(LayerSettings->CollisionLayers.Num() < 8))
	{
		return;
	}

	CollisionFlags.Reset();
	TMap<FName, int32> LayerByIndex;
	for (const FMCCollisionLayerInfo& CollisionLayer : LayerSettings->CollisionLayers)
	{
		uint8 CollisionFlag = 0;
		for (const FName CollidesWith : CollisionLayer.CollidesWith)
		{
			if (!LayerByIndex.Contains(CollidesWith))
			{
				TOptional<int32> OptionalIndex = LayerSettings->GetCollisionIndexWithName(CollidesWith);
				if (!ensureMsgf(OptionalIndex, TEXT("Invalid layer")))
				{
					continue;
				}
				LayerByIndex.Add(CollidesWith, *OptionalIndex);
			}

			CollisionFlag |= (1 << LayerByIndex[CollidesWith]);
		}
		CollisionFlags.Add(CollisionFlag);
	}
}

