#include "Handlers/MRBMassReplicationHandlers.h"


void FMRBMassReplicationTransform::AddEntity(const int32 EntityIdx, FMRBReplicatedPositionWithTimestamp& ReplicatedPosition) const
{
	ReplicatedPosition.Position = TransformList[EntityIdx].GetTransform().GetLocation();
	ReplicatedPosition.Timestamp = 0.0f;
}
