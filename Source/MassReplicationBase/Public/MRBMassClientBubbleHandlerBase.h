#pragma once

#include "MassClientBubbleHandler.h"
#include "CoreMinimal.h"

template<typename AgentArrayItem>
class TMRBMassClientBubbleHandlerBase : public TClientBubbleHandlerBase<AgentArrayItem>
{
public:
#if UE_REPLICATION_COMPILE_SERVER_CODE
	TArray<FMassAgentLookupData>& GetAgentLookupArray() { return this->AgentLookupArray; }

	TArray<AgentArrayItem>* GetAgents() { return this->Agents; }

	FMassClientBubbleSerializerBase* GetSerializer() { return this->Serializer; }

#endif // UE_REPLICATION_COMPILE_SERVER_CODE
};