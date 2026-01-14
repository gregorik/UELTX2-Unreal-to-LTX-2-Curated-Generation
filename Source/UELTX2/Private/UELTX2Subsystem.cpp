#include "UELTX2Subsystem.h"
#include "UELTX2Settings.h"

// --- CORE / HTTP ---
#include "HttpModule.h" 
#include "Interfaces/IHttpResponse.h"
#include "UObject/UnrealType.h" // Required for Reflection (FProperty)

// --- ASSET TOOLS ---
#include "AssetToolsModule.h"
#include "AutomatedAssetImportData.h"
#include "Factories/MaterialFactoryNew.h"

// --- MEDIA ---
#include "MediaPlayer.h"
#include "MediaTexture.h"
#include "FileMediaSource.h"

// --- MATERIAL EXPRESSIONS ---
#include "Materials/Material.h"
#include "Materials/MaterialExpressionTextureSample.h"
#include "Materials/MaterialExpressionMultiply.h"
#include "Materials/MaterialExpressionParticleColor.h"

// --- SEQUENCER ---
#include "LevelSequence.h"
#include "MovieScene.h"
#include "MovieSceneMediaTrack.h"
#include "MovieSceneMediaSection.h"
#include "Channels/MovieSceneChannelProxy.h"

// --- NIAGARA ---
#include "NiagaraSystem.h"


// =============================================================================
// HELPER: REFLECTION SETTER (Bypasses Private Access Restrictions)
// =============================================================================
namespace UELTX2Utils
{
    // Use Reflection to set Enum properties (BlendMode, ShadingModel, TwoSided) 
    // that are private in UE 5.4+
    template <typename TEnum>
    void SetEnumProperty(UObject* Target, FName PropName, TEnum Value)
    {
        if (!Target) return;

        // Find property by name on the UMaterial class
        if (FProperty* Prop = Target->GetClass()->FindPropertyByName(PropName))
        {
            // Case 1: TEnumAsByte (Stored as Byte) - Used for BlendMode/ShadingModel
            if (FByteProperty* ByteProp = CastField<FByteProperty>(Prop))
            {
                // FIX: Use SetIntPropertyValue instead of SetNumericPropertyValue
                void* ValuePtr = ByteProp->ContainerPtrToValuePtr<void>(Target);
                ByteProp->SetIntPropertyValue(ValuePtr, (int64)Value);
            }
            // Case 2: Clean Enum (Stored as Enum)
            else if (FEnumProperty* EnumProp = CastField<FEnumProperty>(Prop))
            {
                void* ValuePtr = EnumProp->ContainerPtrToValuePtr<void>(Target);
                EnumProp->GetUnderlyingProperty()->SetIntPropertyValue(ValuePtr, (int64)Value);
            }
        }
    }

    void SetBoolProperty(UObject* Target, FName PropName, bool Value)
    {
        if (!Target) return;
        if (FBoolProperty* BoolProp = CastField<FBoolProperty>(Target->GetClass()->FindPropertyByName(PropName)))
        {
            void* ValuePtr = BoolProp->ContainerPtrToValuePtr<void>(Target);
            BoolProp->SetPropertyValue(ValuePtr, Value);
        }
    }

    void SetObjectProperty(UObject* Target, FName PropName, UObject* Value)
    {
        if (!Target) return;
        if (FObjectProperty* ObjProp = CastField<FObjectProperty>(Target->GetClass()->FindPropertyByName(PropName)))
        {
            void* ValuePtr = ObjProp->ContainerPtrToValuePtr<void>(Target);
            ObjProp->SetObjectPropertyValue(ValuePtr, Value);
        }
    }
}

// =============================================================================
// SUBSYSTEM IMPLEMENTATION
// =============================================================================

void UUELTX2Subsystem::GenerateVideo(FString Prompt, FString SourceImagePath)
{
    if (!Prompt.IsEmpty()) CachedPrompt = Prompt;

    bool bIsI2V = !SourceImagePath.IsEmpty();
    FString TemplateName = bIsI2V ? TEXT("LTX2_I2V.json") : TEXT("LTX2_T2V.json");
    FString PluginContentDir = FPaths::ProjectPluginsDir() / TEXT("UELTX2/Content/Templates/");
    FString TemplateFile = PluginContentDir / TemplateName;

    FString JsonString;
    if (!FFileHelper::LoadFileToString(JsonString, *TemplateFile))
    {
        UE_LOG(LogTemp, Error, TEXT("UELTX2: Template not found at %s"), *TemplateFile);
        return;
    }

    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);
    if (!FJsonSerializer::Deserialize(Reader, JsonObject)) return;

    // Inject Data
    if (JsonObject->HasField(TEXT("6")))
        JsonObject->GetObjectField(TEXT("6"))->GetObjectField(TEXT("inputs"))->SetStringField(TEXT("text"), Prompt);

    if (bIsI2V && JsonObject->HasField(TEXT("12")))
    {
        FString CleanPath = SourceImagePath.Replace(TEXT("/"), TEXT("\\"));
        JsonObject->GetObjectField(TEXT("12"))->GetObjectField(TEXT("inputs"))->SetStringField(TEXT("image"), CleanPath);
    }

    if (JsonObject->HasField(TEXT("9")))
    {
        JsonObject->GetObjectField(TEXT("9"))->GetObjectField(TEXT("inputs"))->SetNumberField(TEXT("seed"), FMath::RandRange(100000, 99999999));
    }

    // Send
    FString OutputString;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);

    // Wrap for ComfyUI API
    TSharedPtr<FJsonObject> FinalPayload = MakeShareable(new FJsonObject);
    FinalPayload->SetObjectField(TEXT("prompt"), JsonObject);
    FJsonSerializer::Serialize(FinalPayload.ToSharedRef(), Writer);

    const UUELTX2Settings* Settings = UUELTX2Settings::Get();
    FString URL = Settings->ComfyURL + TEXT("/prompt");

    FHttpRequestPtr Request = FHttpModule::Get().CreateRequest();
    Request->OnProcessRequestComplete().BindUObject(this, &UUELTX2Subsystem::OnResponseReceived);
    Request->SetURL(URL);
    Request->SetVerb(TEXT("POST"));
    Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
    Request->SetContentAsString(OutputString);
    Request->ProcessRequest();

    UE_LOG(LogTemp, Log, TEXT("UELTX2: Request Sent."));

    // NEW: Reset Timer to poll for the result
    if (UWorld* World = GEditor->GetEditorWorldContext().World())
    {
        // Check every 1.0 seconds
        World->GetTimerManager().SetTimer(PollingTimer, this, &UUELTX2Subsystem::CheckStatus, 1.0f, true);
        UE_LOG(LogTemp, Log, TEXT("UELTX2: Polling started for output file..."));
    }
}

void UUELTX2Subsystem::OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
    if (bWasSuccessful && Response.IsValid())
    {
        UE_LOG(LogTemp, Log, TEXT("UELTX2 ID: %s"), *Response->GetContentAsString());
    }
}

void UUELTX2Subsystem::ImportVideoAsset(FString AbsoluteVideoPath)
{
#if WITH_EDITOR
    if (AbsoluteVideoPath.IsEmpty()) return;

    FString DestinationPath = UUELTX2Settings::Get()->DefaultImportPath;
    FString FileName = FPaths::GetBaseFilename(AbsoluteVideoPath);
    FAssetToolsModule& AssetToolsModule = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools");

    UAutomatedAssetImportData* ImportData = NewObject<UAutomatedAssetImportData>();
    ImportData->bReplaceExisting = true;
    ImportData->DestinationPath = DestinationPath;
    ImportData->Filenames.Add(AbsoluteVideoPath);
    ImportData->FactoryName = TEXT("FileMediaSourceFactory");

    TArray<UObject*> ImportedAssets = AssetToolsModule.Get().ImportAssetsAutomated(ImportData);

    if (ImportedAssets.Num() > 0)
    {
        if (UFileMediaSource* MediaSource = Cast<UFileMediaSource>(ImportedAssets[0]))
        {
            FString PlayerName = FileName + TEXT("_Player");
            UMediaPlayer* MediaPlayer = Cast<UMediaPlayer>(AssetToolsModule.Get().CreateAsset(
                PlayerName, DestinationPath, UMediaPlayer::StaticClass(), nullptr));

            FString TextureName = FileName + TEXT("_Tex");
            UMediaTexture* MediaTexture = Cast<UMediaTexture>(AssetToolsModule.Get().CreateAsset(
                TextureName, DestinationPath, UMediaTexture::StaticClass(), nullptr));

            if (MediaPlayer && MediaTexture)
            {
                // Note: SetLooping is the public helper for the protected Loop property
                MediaPlayer->SetLooping(true);
                MediaPlayer->PlayOnOpen = true; // Auto-play
                MediaTexture->SetMediaPlayer(MediaPlayer);
                MediaTexture->UpdateResource();
                MediaPlayer->OpenSource(MediaSource);
            }

            // Create Standard Material (I2V)
            if (MediaTexture)
            {
                FString MatName = FileName + TEXT("_Mat");
                UMaterialFactoryNew* MatFactory = NewObject<UMaterialFactoryNew>();
                UMaterial* NewMat = Cast<UMaterial>(AssetToolsModule.Get().CreateAsset(
                    MatName, DestinationPath, UMaterial::StaticClass(), MatFactory));

                if (NewMat)
                {
                    auto* TextureNode = NewObject<UMaterialExpressionTextureSample>(NewMat);
                    TextureNode->Texture = MediaTexture;
                    TextureNode->SamplerType = SAMPLERTYPE_Color;
                    TextureNode->MaterialExpressionEditorX = -200;

                    NewMat->GetExpressionCollection().AddExpression(TextureNode);
                    NewMat->GetEditorOnlyData()->EmissiveColor.Expression = TextureNode;

                    // FIXED: Use Reflection for Shading Model
                    UELTX2Utils::SetEnumProperty(NewMat, FName("ShadingModel"), MSM_Unlit);

                    NewMat->PostEditChange();
                }

                // Create VFX Assets (Workflow 3)
                CreateVFXAssets(AbsoluteVideoPath, DestinationPath, MediaTexture);
            }

            // Create Animatic (Workflow 2)
            CreateAnimaticSequence(AbsoluteVideoPath, DestinationPath, MediaTexture);
        }
    }

    OnGenerationComplete.Broadcast(DestinationPath);
#endif
}

void UUELTX2Subsystem::CreateAnimaticSequence(FString VideoPath, FString DestinationPath, UMediaTexture* MediaTex)
{
#if WITH_EDITOR
    FAssetToolsModule& AssetToolsModule = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools");
    FString FileName = FPaths::GetBaseFilename(VideoPath);
    FString SeqName = FileName + TEXT("_Animatic");

    // Pass nullptr for factory
    ULevelSequence* LevelSeq = Cast<ULevelSequence>(AssetToolsModule.Get().CreateAsset(
        SeqName, DestinationPath, ULevelSequence::StaticClass(), nullptr));

    if (LevelSeq)
    {
        UMovieScene* MovieScene = LevelSeq->GetMovieScene();
        // Requires "MovieSceneMediaTrack.h"
        UMovieSceneMediaTrack* MediaTrack = MovieScene->AddTrack<UMovieSceneMediaTrack>();

        if (MediaTrack)
        {
            // Requires "MovieSceneMediaSection.h"
            UMovieSceneMediaSection* MediaSection = Cast<UMovieSceneMediaSection>(MediaTrack->CreateNewSection());

            FString MediaSourcePath = DestinationPath / FileName + TEXT(".") + FileName;
            UFileMediaSource* MediaSourceAsset = LoadObject<UFileMediaSource>(nullptr, *MediaSourcePath);
            if (MediaSourceAsset && MediaSection)
            {
                // 1. Set The Source (File)
                MediaSection->SetMediaSource(MediaSourceAsset);

                // 2. THE FIX: Set The Sink via Reflection
                if (MediaTex)
                {
                    // "MediaTexture" is the internal FName of the property in MovieSceneMediaSection.h
                    UELTX2Utils::SetObjectProperty(MediaSection, FName("MediaTexture"), MediaTex);
                }

                int32 Duration = 120; // ~5 seconds
                MediaSection->SetRange(TRange<FFrameNumber>(FFrameNumber(0), FFrameNumber(120 * 24000)));
                MediaTrack->AddSection(*MediaSection);

                MovieScene->SetPlaybackRange(0, Duration);
                UE_LOG(LogTemp, Log, TEXT("UELTX2: Animatic Created with Texture Link: %s"), *SeqName);
            }
        }
    }
#endif
}

void UUELTX2Subsystem::CreateVFXAssets(FString VideoPath, FString DestinationPath, UMediaTexture* MediaTex)
{
#if WITH_EDITOR
    FAssetToolsModule& AssetToolsModule = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools");
    FString FileName = FPaths::GetBaseFilename(VideoPath);

    FString MatName = FileName + TEXT("_VFXMat");
    UMaterialFactoryNew* MatFactory = NewObject<UMaterialFactoryNew>();
    UMaterial* VFXMat = Cast<UMaterial>(AssetToolsModule.Get().CreateAsset(
        MatName, DestinationPath, UMaterial::StaticClass(), MatFactory));

    if (VFXMat)
    {
        // FIXED: Use Reflection for Private Properties (The "C2039" Fix)
        UELTX2Utils::SetEnumProperty(VFXMat, FName("ShadingModel"), MSM_Unlit);
        UELTX2Utils::SetEnumProperty(VFXMat, FName("BlendMode"), BLEND_Additive);
        UELTX2Utils::SetBoolProperty(VFXMat, FName("TwoSided"), true);

        // Nodes
        auto* TexNode = NewObject<UMaterialExpressionTextureSample>(VFXMat);
        TexNode->Texture = MediaTex;
        TexNode->SamplerType = SAMPLERTYPE_Color;
        TexNode->MaterialExpressionEditorX = -300;

        auto* ColorNode = NewObject<UMaterialExpressionParticleColor>(VFXMat);
        ColorNode->MaterialExpressionEditorX = -300;
        ColorNode->MaterialExpressionEditorY = 200;

        auto* MultiplyNode = NewObject<UMaterialExpressionMultiply>(VFXMat);
        MultiplyNode->A.Expression = TexNode;
        MultiplyNode->B.Expression = ColorNode;

        VFXMat->GetExpressionCollection().AddExpression(TexNode);
        VFXMat->GetExpressionCollection().AddExpression(ColorNode);
        VFXMat->GetExpressionCollection().AddExpression(MultiplyNode);

        VFXMat->GetEditorOnlyData()->EmissiveColor.Expression = MultiplyNode;
        VFXMat->GetEditorOnlyData()->Opacity.Expression = TexNode;

        VFXMat->PostEditChange();
    }

    // Check for Template (Optional)
    FString TemplatePath = TEXT("/UELTX2/Templates/NS_LTX2_Template");
    if (UNiagaraSystem* TemplateSys = LoadObject<UNiagaraSystem>(nullptr, *TemplatePath))
    {
        AssetToolsModule.Get().DuplicateAsset(FileName + TEXT("_VFX"), DestinationPath, TemplateSys);
    }
#endif
}

void UUELTX2Subsystem::CheckStatus()
{
        // We look for files matching the prefix defined in our JSON (UELTX2_Output)
        // and check if they are newer than the generation start time.

        FString OutputDir = UUELTX2Settings::Get()->ComfyOutputDir;
        FString SearchPattern = OutputDir / TEXT("UELTX2_Output_*.mp4"); // Matches format from JSON

        TArray<FString> FoundFiles;
        IFileManager::Get().FindFiles(FoundFiles, *OutputDir, TEXT(".mp4"));

        if (FoundFiles.Num() == 0) return;

        // Sort to find the newest file
        FoundFiles.Sort();
        FString NewestFile = OutputDir / FoundFiles.Last();

        FFileStatData StatData = IFileManager::Get().GetStatData(*NewestFile);

        // Logic: If the file was modified within the last 5 seconds, it's likely our new video.
        // (A more robust way is querying Comfy History, but this is sufficient for single-user local setups)
        FDateTime Now = FDateTime::Now();
        if ((Now - StatData.ModificationTime).GetTotalSeconds() < 5.0)
        {
            // Wait a moment for file lock to release (writing finished)
            if (UWorld* World = GEditor->GetEditorWorldContext().World())
            {
                World->GetTimerManager().ClearTimer(PollingTimer); // Stop Polling

                // Trigger Import
                ImportVideoAsset(NewestFile);

                UE_LOG(LogTemp, Warning, TEXT("UELTX2: File Detected! Importing: %s"), *NewestFile);
            }
        }
}
