#include "UELTX2Style.h"
#include "Styling/SlateStyleRegistry.h"
#include "Framework/Application/SlateApplication.h"
#include "Slate/SlateGameResources.h"
#include "Interfaces/IPluginManager.h"
#include "Styling/SlateStyleMacros.h"

#define RootToContentDir Style->RootToContentDir

TSharedPtr<FSlateStyleSet> FUELTX2Style::StyleInstance = nullptr;

void FUELTX2Style::Initialize()
{
	if (!StyleInstance.IsValid())
	{
		StyleInstance = Create();
		FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
	}
}

void FUELTX2Style::Shutdown()
{
	if (StyleInstance.IsValid())
	{
		FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
		ensure(StyleInstance.IsUnique());
		StyleInstance.Reset();
	}
}

FName FUELTX2Style::GetStyleSetName()
{
	static FName StyleSetName(TEXT("UELTX2Style"));
	return StyleSetName;
}

const ISlateStyle& FUELTX2Style::Get()
{
	return *StyleInstance;
}

void FUELTX2Style::ReloadTextures()
{
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().GetRenderer()->ReloadTextureResources();
	}
}

const FVector2D Icon16x16(16.0f, 16.0f);
const FVector2D Icon20x20(20.0f, 20.0f);
const FVector2D Icon40x40(40.0f, 40.0f);

TSharedRef<FSlateStyleSet> FUELTX2Style::Create()
{
	TSharedRef<FSlateStyleSet> Style = MakeShareable(new FSlateStyleSet(GetStyleSetName()));

	FString PluginBaseDir = IPluginManager::Get().FindPlugin("UELTX2")->GetBaseDir();
	Style->SetContentRoot(PluginBaseDir / TEXT("Resources"));

	// 1. Toolbar Icon (40x40)
	Style->Set("UELTX2.OpenUELTX2Window", new IMAGE_BRUSH(TEXT("ButtonIcon"), Icon40x40));

	// 2. NEW: Context Menu Icon (16x16) - Uses the same image file but scaled down
	Style->Set("UELTX2.ContextMenuIcon", new IMAGE_BRUSH(TEXT("ButtonIcon"), Icon16x16));

	// 3. Plugin Icon (128x128)
	Style->Set("UELTX2.PluginIcon", new IMAGE_BRUSH(TEXT("Icon128"), FVector2D(128.0f, 128.0f)));

	return Style;
}

#undef RootToContentDir