#include "UELTX2Module.h"

#define LOCTEXT_NAMESPACE "FUELTX2Module"

// Define the log category declared in the header
DEFINE_LOG_CATEGORY(LogUELTX2);

void FUELTX2Module::StartupModule()
{
	// This code will execute after your module is loaded into memory.
	// The exact timing is specified in the .uplugin (usually 'Default').
	
	UE_LOG(LogUELTX2, Log, TEXT("UELTX2 Runtime Module Started. Bridge functionality ready."));
}

void FUELTX2Module::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.
	// For modules that support dynamic reloading, we call this function before unloading the module.
	
	UE_LOG(LogUELTX2, Log, TEXT("UELTX2 Runtime Module Shutting Down."));
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FUELTX2Module, UELTX2)