#include "UELTX2Panel.h"
#include "Engine/Engine.h"
#include "Engine/Texture2D.h"
#include "Editor.h" 
#include "ContentBrowserModule.h" // Required
#include "IContentBrowserSingleton.h" // Required
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "ImageUtils.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "Modules/ModuleManager.h"

void UUELTX2Panel::NativeConstruct()
{
    Super::NativeConstruct();

    // RESTORE STATE:
    if (GEngine && InputPrompt)
    {
        UUELTX2Subsystem* LTXSubsystem = GEngine->GetEngineSubsystem<UUELTX2Subsystem>();
        if (LTXSubsystem && !LTXSubsystem->CachedPrompt.IsEmpty())
        {
            InputPrompt->SetText(FText::FromString(LTXSubsystem->CachedPrompt));
        }
    }

    if (BtnGenerate)
        BtnGenerate->OnClicked.AddDynamic(this, &UUELTX2Panel::OnGenerateClicked);

    if (BtnUseSelected)
        BtnUseSelected->OnClicked.AddDynamic(this, &UUELTX2Panel::OnUseSelectedClicked);

    if (GEngine)
    {
        UUELTX2Subsystem* LTXSubsystem = GEngine->GetEngineSubsystem<UUELTX2Subsystem>();
        if (LTXSubsystem)
        {
            LTXSubsystem->OnGenerationComplete.AddDynamic(this, &UUELTX2Panel::OnGenerationCompleted);
        }
    }
}

void UUELTX2Panel::OnUseSelectedClicked()
{
    // FIX: Safer way to get selection via Module
    FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
    IContentBrowserSingleton& ContentBrowserSingleton = ContentBrowserModule.Get();

    TArray<FAssetData> SelectedAssets;
    ContentBrowserSingleton.GetSelectedAssets(SelectedAssets);

    UTexture2D* Texture = nullptr;

    for (const FAssetData& AssetData : SelectedAssets)
    {
        // Safe cast to find first texture
        if (UTexture2D* Candidate = Cast<UTexture2D>(AssetData.GetAsset()))
        {
            Texture = Candidate;
            break;
        }
    }

    if (Texture)
    {
        FString TempDir = FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir() / TEXT("UELTX2_Temp"));
        IFileManager::Get().MakeDirectory(*TempDir, true);
        FString OutputFilename = TempDir / (Texture->GetName() + TEXT("_Input.png"));

        // FIX: Verify Source Data exists before accessing
        if (Texture->Source.IsValid())
        {
            FImage SourceImage;
            // Use GetMipImage, but guard it
            if (Texture->Source.GetMipImage(SourceImage, 0))
            {
                IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
                TSharedPtr<IImageWrapper> ImageWrapper = ImageWrapperModule.CreateImageWrapper(EImageFormat::PNG);

                if (ImageWrapper.IsValid() && ImageWrapper->SetRaw(SourceImage.RawData.GetData(), SourceImage.RawData.Num(), SourceImage.SizeX, SourceImage.SizeY, ERGBFormat::BGRA, 8))
                {
                    const TArray64<uint8>& CompressedData = ImageWrapper->GetCompressed();
                    if (FFileHelper::SaveArrayToFile(CompressedData, *OutputFilename))
                    {
                        SelectedImagePath = OutputFilename;

                        if (ImgPreview)
                        {
                            ImgPreview->SetBrushFromTexture(Texture);
                            ImgPreview->SetVisibility(ESlateVisibility::Visible);
                        }

                        if (TxtStatus) TxtStatus->SetText(FText::FromString(FString::Printf(TEXT("Selected: %s"), *Texture->GetName())));
                        return;
                    }
                }
            }
            else
            {
                if (TxtStatus) TxtStatus->SetText(FText::FromString(TEXT("Error: Texture has no Source Data (Is it cooked?)")));
            }
        }
    }
    else
    {
        SelectedImagePath.Empty();
        if (ImgPreview) ImgPreview->SetVisibility(ESlateVisibility::Collapsed);
        if (TxtStatus) TxtStatus->SetText(FText::FromString(TEXT("Please select a Texture in Content Browser.")));
    }
}

void UUELTX2Panel::OnGenerateClicked()
{
    if (!InputPrompt) return;

    FString PromptText = InputPrompt->GetText().ToString();
    if (PromptText.IsEmpty())
    {
        if (TxtStatus) TxtStatus->SetText(FText::FromString(TEXT("Error: Prompt is empty.")));
        return;
    }

    if (GEngine)
    {
        UUELTX2Subsystem* LTXSubsystem = GEngine->GetEngineSubsystem<UUELTX2Subsystem>();
        if (LTXSubsystem)
        {
            if (TxtStatus) TxtStatus->SetText(FText::FromString(TEXT("Sending Request to LTX-2...")));

            if (!SelectedImagePath.IsEmpty())
            {
                LTXSubsystem->GenerateVideo(PromptText, SelectedImagePath);
            }
            else
            {
                LTXSubsystem->GenerateVideo(PromptText, FString());
            }
        }
    }
}

void UUELTX2Panel::OnGenerationCompleted(FString OutputPath)
{
    if (TxtStatus)
    {
        TxtStatus->SetText(FText::FromString(TEXT("Import Complete! Check Content Browser.")));
    }

    // Reset state
    SelectedImagePath.Empty();
    if (ImgPreview) ImgPreview->SetVisibility(ESlateVisibility::Collapsed);
}