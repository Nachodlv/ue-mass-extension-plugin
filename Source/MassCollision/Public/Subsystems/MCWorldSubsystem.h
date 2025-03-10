#pragma once

// UE Includes
#include "CoreMinimal.h"
#include "MassEntityTypes.h"
#include "Spatial/SparseDynamicOctree3.h"
#include "Subsystems/WorldSubsystem.h"

#include "MCWorldSubsystem.generated.h"

struct FMCEntityCollisionData
{
	FMCEntityCollisionData (uint32 InCollisionLayerFlag, int32 InOctreeIndex, const FBoxSphereBounds& InBounds)
		: CollisionLayerFlag(InCollisionLayerFlag), OctreeIndex(InOctreeIndex), Bounds(InBounds) {}

	/** The entity collision layer */
	uint32 CollisionLayerFlag;

	/** Object ID for this entity inside the octree */
	int32 OctreeIndex = INDEX_NONE;

	FBoxSphereBounds Bounds;
};

USTRUCT()
struct FMCCollisionLayerInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	FName LayerName;

	/** The layers to which this layer will collide */
	UPROPERTY(EditAnywhere, meta = (GetOptions = "GetCollisionLayerNames"))
	TArray<FName> CollidesWith;

	bool operator==(const FMCCollisionLayerInfo& Other) const;
};

/** Contains the collision layers present in the project and the interactions between them */
UCLASS(Config = Game, DisplayName = "Mass Collision Layers", DefaultConfig)
class MASSCOLLISION_API UMCCollisionLayersSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	TOptional<int32> GetCollisionIndexWithName(const FName& LayerName) const;
	
	UPROPERTY(EditAnywhere, Config, meta = (TitleProperty = "LayerName", NoElementDuplicate))
	TArray<FMCCollisionLayerInfo> CollisionLayers;

protected:
	UFUNCTION(CallInEditor)
	static TArray<FName> GetCollisionLayerNames();
};

UCLASS()
class MASSCOLLISION_API UMCWorldSubsystem : public UWorldSubsystem, public FTickableGameObject
{
	GENERATED_BODY()

public:
	bool HasCollision(const FMassEntityHandle& EntityHandle);

	void AddCollision(const FMassEntityHandle& EntityHandle, const FBoxSphereBounds& Bounds, int32 CollisionLayerIndex);

	void UpdateCollision(const FMassEntityHandle& EntityHandle, const FBoxSphereBounds& NewBounds);

	void RemoveCollision(const FMassEntityHandle& EntityHandle);

	void RetrieveCollisions(const FBoxSphereBounds& SearchBounds, int32 CollisionLayerIndex, TFunctionRef<void(const FMassEntityHandle&)> ObjectIDFunc) const;

	void RetrieveCollisionsByFlag(const FBoxSphereBounds& SearchBounds, uint8 CollisionFlag, TFunctionRef<void(const FMassEntityHandle&)> ObjectIDFunc) const;

	virtual TStatId GetStatId() const override;

	virtual bool IsAllowedToTick() const override;

	virtual void Tick(float DeltaTime) override;
	
protected:
	virtual void PostInitialize() override;

private:
	/** Guard to read and write from the collision octree */
	UE_MT_DECLARE_RW_ACCESS_DETECTOR(OctreeAccessDetector);
	
	/** Stores the entities collisions */
	UE::Geometry::FSparseDynamicOctree3 CollisionOctree;

	/** Collision data for each entity present in the world */
	TMap<FMassEntityHandle, FMCEntityCollisionData> EntityCollisionDataByMassID;
	
	/** Associates the octree object id with the mass entity handle for faster searches */
	TMap<int32, FMassEntityHandle> EntitiesByOctreeElementId;

	/** The collision flag for each layer.
	 * The indexes of each element correspond to the indexes on UMCCollisionLayersSettings::CollisionLayers.
	 * The flag contains the indexes of collision layers. */
	TArray<uint32> CollisionFlags;
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
