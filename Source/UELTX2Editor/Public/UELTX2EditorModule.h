#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

// Declare a log category specifically for Editor actions
DECLARE_LOG_CATEGORY_EXTERN(LogUELTX2Editor, Log, All);

class FUELTX2EditorModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
    
    /**
     * Helper to hook into the UI (ToolMenus) during startup.
     */
    void RegisterMenus();
};