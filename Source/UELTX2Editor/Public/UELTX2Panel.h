#pragma once

#include "CoreMinimal.h"
#include "EditorUtilityWidget.h"
#include "UELTX2Subsystem.h"
#include "Components/Button.h"
#include "Components/MultiLineEditableTextBox.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/Border.h"
#include "UELTX2Panel.generated.h"

UCLASS(BlueprintType, Blueprintable)
class UELTX2EDITOR_API UUELTX2Panel : public UEditorUtilityWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;

protected:
    // --- UI Bindings ---

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UMultiLineEditableTextBox> InputPrompt;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UButton> BtnGenerate;

    // NEW: Button to grab texture from Content Browser
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UButton> BtnUseSelected;

    // NEW: Small preview of the selected source image
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UImage> ImgPreview;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> TxtStatus;

    // --- Internal State ---
    FString SelectedImagePath;

    // --- Logic ---
    UFUNCTION()
    void OnGenerateClicked();

    UFUNCTION()
    void OnUseSelectedClicked(); // The new logic

    UFUNCTION()
    void OnGenerationCompleted(FString OutputPath);
};