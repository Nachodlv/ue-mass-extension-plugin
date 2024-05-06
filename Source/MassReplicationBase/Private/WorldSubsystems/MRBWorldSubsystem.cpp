#include "WorldSubsystems/MRBWorldSubsystem.h"

// UE Includes
#include "MassReplicationSubsystem.h"

// MRB Includes
#include "MRBMassClientBubbleInfo.h"

void UMRBWorldSubsystem::PostInitialize()
{
	Super::PostInitialize();
	
	const UWorld* World = GetWorld();
	UMassReplicationSubsystem* ReplicationSubsystem = UWorld::GetSubsystem<UMassReplicationSubsystem>(World);
	check(ReplicationSubsystem);

	for (const FSoftClassPath& ClientBubbleToRegister : MassClientBubblesToRegister)
	{
		if (!ClientBubbleToRegister.IsValid())
		{
			continue;
		}
		TSubclassOf<AMassClientBubbleInfoBase> ClientBubbleClass = LoadClass<AMassClientBubbleInfoBase>(nullptr, *ClientBubbleToRegister.ToString());
		ReplicationSubsystem->RegisterBubbleInfoClass(ClientBubbleClass);
	}
}
