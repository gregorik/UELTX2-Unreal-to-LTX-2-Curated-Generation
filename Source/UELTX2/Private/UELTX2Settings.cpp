#include "UELTX2Settings.h"

UUELTX2Settings::UUELTX2Settings()
{
    // These define where it appears in the Project Settings UI
    // (e.g. Project Settings -> Game -> LTX-2 Generation)
    CategoryName = TEXT("Game");
    SectionName = TEXT("UELTX2 Generation");

    // Default to the standard local ComfyUI address
    ComfyURL = TEXT("http://127.0.0.1:8188");

    // Add default guess
    ComfyOutputDir = TEXT("C:/ComfyUI/output/");

    // Check status every 1.0 seconds
    PollingInterval = 1.0f;

    // Default folder structure for organized assets
    DefaultImportPath = TEXT("/Game/UELTX2_Generations/");

    // Defaults to true so artists get the asset immediately
    bAutoImport = true;
}

const UUELTX2Settings* UUELTX2Settings::Get()
{
    // Helper to get the CDO (Class Default Object) efficiently
    return GetDefault<UUELTX2Settings>();
          
}