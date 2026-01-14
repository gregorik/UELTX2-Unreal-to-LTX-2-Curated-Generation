#include "UELTX2AssetActions.h"
#include "UELTX2Subsystem.h"
#include "UELTX2Style.h"
#include "ToolMenus.h"
#include "ContentBrowserModule.h"
#include "ContentBrowserMenuContexts.h" 
#include "Engine/Texture2D.h"
#include "ImageUtils.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "Modules/ModuleManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"

#define LOCTEXT_NAMESPACE "FUELTX2AssetActions"

void FUELTX2AssetActions::Register()
{
    UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("ContentBrowser.AssetContextMenu.Texture");
    if (!Menu) return;

    FToolMenuSection& Section = Menu->FindOrAddSection("UELTX2Actions");
    Section.Label = LOCTEXT("UELTX2SectionLabel", "Generative AI");

    Section.AddMenuEntry(
        "UELTX2_AnimateTexture",
        LOCTEXT("UELTX2_AnimateTextureLabel", "Animate with LTX-2 (I2V)"),
        LOCTEXT("UELTX2_AnimateTextureTooltip", "Generates a video variation using LTX-2 Image-to-Video."),
        // Custom 16x16 Icon
        FSlateIcon(FUELTX2Style::GetStyleSetName(), "UELTX2.ContextMenuIcon"),
        FToolMenuExecuteAction::CreateStatic(&FUELTX2AssetActions::ExecuteAnimateTexture)
    );
}

void FUELTX2AssetActions::Unregister()
{
    if (UToolMenus* ToolMenus = UToolMenus::TryGet())
    {
        ToolMenus->RemoveSection("ContentBrowser.AssetContextMenu.Texture", "UELTX2Actions");
    }
}

void FUELTX2AssetActions::ExecuteAnimateTexture(const FToolMenuContext& MenuContext)
{
    // 1. Get Selection
    UContentBrowserAssetContextMenuContext* Context = MenuContext.FindContext<UContentBrowserAssetContextMenuContext>();
    if (!Context || Context->SelectedAssets.Num() == 0) return;

    // 2. Prepare Export
    TArray<FString> ProcessedPaths; // Defined here to fix your error

    FString TempDir = FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir() / TEXT("UELTX2_Temp"));
    IFileManager::Get().MakeDirectory(*TempDir, true);

    IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));

    // 3. Process each asset
    for (const FAssetData& AssetData : Context->SelectedAssets)
    {
        UTexture2D* Texture = Cast<UTexture2D>(AssetData.GetAsset());
        if (!Texture) continue;

        FString OutputFilename = TempDir / (AssetData.AssetName.ToString() + TEXT(".png"));

        // Safe Export Logic
        if (Texture->Source.IsValid())
        {
            FImage SourceImage;
            if (Texture->Source.GetMipImage(SourceImage, 0))
            {
                TSharedPtr<IImageWrapper> ImageWrapper = ImageWrapperModule.CreateImageWrapper(EImageFormat::PNG);

                // Usually BGRA8 for Source Data
                if (ImageWrapper.IsValid() && ImageWrapper->SetRaw(SourceImage.RawData.GetData(), SourceImage.RawData.Num(), SourceImage.SizeX, SourceImage.SizeY, ERGBFormat::BGRA, 8))
                {
                    const TArray64<uint8>& CompressedData = ImageWrapper->GetCompressed();
                    if (FFileHelper::SaveArrayToFile(CompressedData, *OutputFilename))
                    {
                        ProcessedPaths.Add(OutputFilename);
                    }
                }
            }
        }
    }

    if (ProcessedPaths.Num() == 0) return;

    // 4. Submit to Subsystem
    UUELTX2Subsystem* LTX2Subsystem = GEngine->GetEngineSubsystem<UUELTX2Subsystem>();
    if (LTX2Subsystem)
    {
        // retrieve last used prompt
        FString TargetPrompt = LTX2Subsystem->CachedPrompt;

        // Fallback default
        if (TargetPrompt.IsEmpty())
        {
            TargetPrompt = TEXT("slow camera movement, dynamic lighting, cinematic, 4k, loopable");
        }

        for (const FString& ImagePath : ProcessedPaths)
        {
            LTX2Subsystem->GenerateVideo(TargetPrompt, ImagePath);
        }

        // Notification
        FNotificationInfo Info(LOCTEXT("GenStarted", "LTX-2 Generation Started"));
        Info.SubText = FText::FromString(FString::Printf(TEXT("Prompt: %s..."), *TargetPrompt.Left(20)));
        Info.ExpireDuration = 3.0f;
        FSlateNotificationManager::Get().AddNotification(Info);
    }
}

#undef LOCTEXT_NAMESPACE