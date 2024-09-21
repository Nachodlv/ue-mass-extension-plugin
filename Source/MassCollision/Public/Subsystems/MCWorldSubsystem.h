#pragma once

#include "CoreMinimal.h"
#include "MassEntityTypes.h"
#include "Spatial/SparseDynamicOctree3.h"
#include "Subsystems/WorldSubsystem.h"
#include "MCWorldSubsystem.generated.h"

UCLASS()
class MASSCOLLISION_API UMCWorldSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	bool HasCollision(const FMassEntityHandle& EntityHandle);

	void AddCollision(const FMassEntityHandle& EntityHandle, const UE::Geometry::FAxisAlignedBox3d& Bounds);

	bool NeedsCollisionUpdate(const FMassEntityHandle& EntityHandle, const UE::Geometry::FAxisAlignedBox3d& NewBounds, uint32& CellIDOut);

	void UpdateCollision(const FMassEntityHandle& EntityHandle, const UE::Geometry::FAxisAlignedBox3d& NewBounds, uint32 CellIDHint = TNumericLimits<uint32>::Max());

	void RemoveCollision(const FMassEntityHandle& EntityHandle);

	void RetrieveCollisions(const UE::Geometry::FAxisAlignedBox3d& SearchBounds,
		TFunctionRef<void(const FMassEntityHandle&)> ObjectIDFunc);

private:
	/** Guard to read and write from the collision octree */
	UE_MT_DECLARE_RW_ACCESS_DETECTOR(OctreeAccessDetector);
	
	/** Stores the entities collisions */
	UE::Geometry::FSparseDynamicOctree3 CollisionOctree;

	/** Collision data for each entity present in the world */
	TMap<int32, FMassEntityHandle> EntityCollisionData;
};

template<>
struct TMassExternalSubsystemTraits<UMCWorldSubsystem> final
{
	enum
	{
		ThreadSafeRead = true,
		ThreadSafeWrite = true,
		GameThreadOnly = false,
	};
};
