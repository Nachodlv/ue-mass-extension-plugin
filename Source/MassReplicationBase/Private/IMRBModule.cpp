#include "IMRBModule.h"

#define LOCTEXT_NAMESPACE "MassReplicationBase"

class FMRBModule : public IMRBModule
{
public:

	/** IModuleInterface implementation */
	void StartupModule() override
	{
		IMRBModule::StartupModule();
	}

	void ShutdownModule() override
	{
		IMRBModule::ShutdownModule();
	}
};

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FMRBModule, MassReplicationBase);