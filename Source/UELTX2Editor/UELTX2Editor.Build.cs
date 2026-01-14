using UnrealBuildTool;

public class UELTX2Editor : ModuleRules
{
	public UELTX2Editor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Projects",
				"InputCore",
				"UnrealEd",
				"ToolMenus",
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				"AssetTools",      // Required for creating Context Menu actions
				"EditorStyle",     // Required for standard Editor icons
				"LevelEditor",     // Required to hook into the main editor window
				"UELTX2",           // LINK: Access the Runtime Subsystem logic
				"Blutility",
				"UMG",
				"UMGEditor",
                "ContentBrowser",
                "ImageWrapper",
				"ImageCore",
                "AssetRegistry"

            }
		);
	}
}