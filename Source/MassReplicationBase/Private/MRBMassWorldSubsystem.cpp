#include "MRBMassWorldSubsystem.h"

#include "MassReplicationSubsystem.h"
#include "MRBMassClientBubbleInfo.h"


void UMRBMassWorldSubsystem::PostInitialize()
{
	Super::PostInitialize();

	UMassReplicationSubsystem* ReplicationSubsystem = UWorld::GetSubsystem<UMassReplicationSubsystem>(GetWorld());
	check(ReplicationSubsystem);
	ReplicationSubsystem->RegisterBubbleInfoClass(AMRBMassClientBubbleInfo::StaticClass());
	
}
