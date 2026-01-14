#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

// Declare a log category for the plugin so we can filter logs easily in the Output Log
// Usage in .cpp: UE_LOG(LogUELTX2, Log, TEXT("Message"));
DECLARE_LOG_CATEGORY_EXTERN(LogUELTX2, Log, All);

class FUELTX2Module : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};