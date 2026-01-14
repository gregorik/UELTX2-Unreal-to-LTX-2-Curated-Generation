// File: Plugins/UELTX2/Source/UELTX2/Public/UELTX2Subsystem.h
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/EngineSubsystem.h"
#include "Interfaces/IHttpRequest.h"
#include "Serialization/JsonSerializer.h"
#include "UELTX2Subsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLTXGenerationComplete, FString, OutputVideoPath);

class UMediaTexture;
/**
 * Main Bridge Logic for communicating with ComfyUI.
 */
UCLASS()
class UELTX2_API UUELTX2Subsystem : public UEngineSubsystem
{
    GENERATED_BODY()

public:
    // Entry point for UI/Blueprints
    UFUNCTION(BlueprintCallable, Category = "LTX2")
    void GenerateVideo(FString Prompt, FString SourceImagePath);
    
    UPROPERTY(BlueprintReadOnly, Category = "LTX2")
    FString CachedPrompt;

    // Event hooks for UI
    UPROPERTY(BlueprintAssignable, Category = "LTX2")
    FOnLTXGenerationComplete OnGenerationComplete;

    void CreateAnimaticSequence(FString VideoPath, FString DestinationPath, UMediaTexture* MediaTex);

    void CreateVFXAssets(FString VideoPath, FString DestinationPath, UMediaTexture* MediaTex);

private:
    void SendPromptToComfy(FString Prompt, FString SourceImagePath);
    
    // HTTP Callback
    void OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
    
    // Asset Import Helper
    void ImportVideoAsset(FString AbsoluteVideoPath);

    // Poll for status
    void CheckStatus();
    
    FTimerHandle PollingTimer;
    FString CurrentPromptID;
};