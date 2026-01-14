#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "UELTX2Settings.generated.h"

/**
 * Configuration for the LTX-2 ComfyUI Bridge.
 * Accessible via Project Settings -> Game -> LTX-2 Generation.
 */
UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "LTX-2 Generation"))
class UELTX2_API UUELTX2Settings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UUELTX2Settings();

	/** 
	 * The base URL of the running ComfyUI server. 
	 * Default: http://127.0.0.1:8188 
	 */
	UPROPERTY(Config, EditAnywhere, Category = "Connection")
	FString ComfyURL;

	/**
	 * How frequently (in seconds) the system checks if the video is ready.
	 * Lower values invoke more network overhead but feel snappier.
	 */
	UPROPERTY(Config, EditAnywhere, Category = "Connection", meta = (ClampMin = "0.5", ClampMax = "10.0"))
	float PollingInterval;

	/** 
	 * The Content Browser path where generated videos will be saved.
	 * Default: /Game/UELTX2_Generations/
	 */
	UPROPERTY(Config, EditAnywhere, Category = "Assets")
	FString DefaultImportPath;

	/** 
	 * If true, the system will automatically import the generated .mp4 as a FileMediaSource asset.
	 */
	UPROPERTY(Config, EditAnywhere, Category = "Assets")
	bool bAutoImport;

	/**
	 * Helper function to access these settings from C++.
	 */
	static const UUELTX2Settings* Get();

	/**
	 * The absolute path to your ComfyUI Output folder.
	 * Example: C:/ComfyUI/output/
	 */
	UPROPERTY(Config, EditAnywhere, Category = "Connection")
	FString ComfyOutputDir;
};