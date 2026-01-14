#include "UELTX2EditorModule.h"
#include "UELTX2AssetActions.h" // <--- CRITICAL ADDITION
#include "UELTX2Style.h"
#include "ToolMenus.h"
#include "LevelEditor.h" 
#include "EditorUtilityWidget.h"
#include "EditorUtilityWidgetBlueprint.h"
#include "EditorUtilitySubsystem.h"
#include "Interfaces/IPluginManager.h"
#include "Styling/SlateIconFinder.h"

#define LOCTEXT_NAMESPACE "FUELTX2EditorModule"

DEFINE_LOG_CATEGORY(LogUELTX2Editor);

void FUELTX2EditorModule::StartupModule()
{
	// 1. Initialize Styles (Icons)
	FUELTX2Style::Initialize();
	FUELTX2Style::ReloadTextures();

	// 2. Register Context Menus (The Fix)
	// We use a callback to ensure the ToolMenus system is fully ready
	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FUELTX2EditorModule::RegisterMenus));

	UE_LOG(LogUELTX2Editor, Log, TEXT("UELTX2 Editor Module Started."));
}

void FUELTX2EditorModule::ShutdownModule()
{
	// 1. Unregister Context Menus
	UToolMenus::UnRegisterStartupCallback(this);
	UToolMenus::UnregisterOwner(this);

	// 2. Unregister Asset Actions (Clean up right-click menu)
	FUELTX2AssetActions::Unregister();

	// 3. Clean up Styles
	FUELTX2Style::Shutdown();

	UE_LOG(LogUELTX2Editor, Log, TEXT("UELTX2 Editor Module Shutting Down."));
}

void FUELTX2EditorModule::RegisterMenus()
{
	FToolMenuOwnerScoped OwnerScoped(this);

	// --- PART A: Register Asset Context Menus ---
	// This adds the "Animate with LTX-2" option to textures
	FUELTX2AssetActions::Register();


	// --- PART B: Register Toolbar & Window Menus ---
	FExecuteAction OpenPluginAction = FExecuteAction::CreateLambda([]() {
		const FString WidgetPath = TEXT("/UELTX2/UI/EUW_UELTX2_Panel.EUW_UELTX2_Panel");
		UObject* WidgetObj = LoadObject<UObject>(nullptr, *WidgetPath);
		if (WidgetObj)
		{
			UEditorUtilityWidgetBlueprint* WidgetBP = Cast<UEditorUtilityWidgetBlueprint>(WidgetObj);
			if (WidgetBP)
			{
				UEditorUtilitySubsystem* Subsystem = GEditor->GetEditorSubsystem<UEditorUtilitySubsystem>();
				if (Subsystem) Subsystem->SpawnAndRegisterTab(WidgetBP);
			}
		}
		else
		{
			UE_LOG(LogUELTX2Editor, Warning, TEXT("Could not find EUW at %s"), *WidgetPath);
			FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(TEXT("Could not find EUW_UELTX2_Panel in Plugins/UELTX2 Content/UI/.")));
		}
		});

	// Main Toolbar
	{
		UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar.PlayToolBar");
		FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("UELTX2Tools");

		FToolMenuEntry ToolbarEntry = FToolMenuEntry::InitToolBarButton(
			"OpenUELTX2Window",
			FUIAction(OpenPluginAction),
			LOCTEXT("UELTX2ButtonLabel", "LTX-2"),
			LOCTEXT("UELTX2ButtonTooltip", "Open the UELTX2 Generation Panel"),
			FSlateIcon(FUELTX2Style::GetStyleSetName(), "UELTX2.OpenUELTX2Window")
		);
		ToolbarEntry.StyleNameOverride = "CalloutToolbar";
		Section.AddEntry(ToolbarEntry);
	}

	// Window Menu
	{
		UToolMenu* WindowMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
		FToolMenuSection& Section = WindowMenu->FindOrAddSection("GenerativeAI");
		Section.Label = LOCTEXT("GenerativeAILabel", "Generative AI");

		Section.AddMenuEntry(
			"OpenUELTX2Window_Menu",
			LOCTEXT("UELTX2MenuLabel", "Open LTX-2 Panel"),
			LOCTEXT("UELTX2MenuTooltip", "Open the LTX-2 Curated Generation Tool"),
			FSlateIcon(FUELTX2Style::GetStyleSetName(), "UELTX2.OpenUELTX2Window"),
			FUIAction(OpenPluginAction)
		);
	}
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FUELTX2EditorModule, UELTX2Editor)