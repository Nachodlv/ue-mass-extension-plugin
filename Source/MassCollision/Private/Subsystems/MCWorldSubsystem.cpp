#include "Subsystems/MCWorldSubsystem.h"

bool UMCWorldSubsystem::HasCollision(const FMassEntityHandle& EntityHandle)
{
	return EntityCollisionData.Find(EntityHandle.Index) != nullptr;
}

void UMCWorldSubsystem::AddCollision(const FMassEntityHandle& EntityHandle,
	const UE::Geometry::FAxisAlignedBox3d& Bounds)
{
	UE_MT_SCOPED_WRITE_ACCESS(OctreeAccessDetector);

	CollisionOctree.InsertObject(EntityHandle.Index, Bounds);
	EntityCollisionData.Add(EntityHandle.Index, EntityHandle);
}

bool UMCWorldSubsystem::NeedsCollisionUpdate(const FMassEntityHandle& EntityHandle,
	const UE::Geometry::FAxisAlignedBox3d& NewBounds, uint32& CellIDOut)
{
	UE_MT_SCOPED_READ_ACCESS(OctreeAccessDetector);

	return CollisionOctree.CheckIfObjectNeedsReinsert(EntityHandle.Index, NewBounds, CellIDOut);
}

void UMCWorldSubsystem::UpdateCollision(const FMassEntityHandle& EntityHandle,
	const UE::Geometry::FAxisAlignedBox3d& NewBounds, uint32 CellIDHint)
{
	UE_MT_SCOPED_WRITE_ACCESS(OctreeAccessDetector);

	CollisionOctree.ReinsertObject(EntityHandle.Index, NewBounds, CellIDHint);
}

void UMCWorldSubsystem::RemoveCollision(const FMassEntityHandle& EntityHandle)
{
	UE_MT_SCOPED_WRITE_ACCESS(OctreeAccessDetector);

	CollisionOctree.RemoveObject(EntityHandle.Index);
	EntityCollisionData.Remove(EntityHandle.Index);
}

void UMCWorldSubsystem::RetrieveCollisions(const UE::Geometry::FAxisAlignedBox3d& SearchBounds,
	TFunctionRef<void(const FMassEntityHandle&)> ObjectIDFunc)
{
	UE_MT_SCOPED_READ_ACCESS(OctreeAccessDetector);

	CollisionOctree.RangeQuery(SearchBounds, [&ObjectIDFunc, this](int32 ObjectID)
	{
		ObjectIDFunc(EntityCollisionData.FindChecked(ObjectID));
	});
}
