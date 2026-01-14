using UnrealBuildTool;

public class UELTX2 : ModuleRules
{
	public UELTX2(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePaths.AddRange(
			new string[] {
				// Public includes if needed
			}
		);
				
		PrivateIncludePaths.AddRange(
			new string[] {
				// Private includes if needed
			}
		);

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"InputCore",
				"HTTP",             // Vital: For sending requests to ComfyUI
				"Json",             // Vital: For parsing the LTX-2 Response
				"JsonUtilities",    // Vital: For constructing the LTX-2 Payload
				"MediaAssets",      // Vital: For handling the output video container
				"DeveloperSettings",
                "MediaCompositing",
				"Niagara"   // Vital: For Projet Settings (Server URL config)
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Projects",         // Required to access Plugin Content directory (Templates)
				"Slate",
				"SlateCore",
				"ImageWrapper",     // Required to save Viewport Screenshots (I2V)
				"RenderCore",       // Required for efficient screen capture
				"RHI",
                "UnrealEd",
				"MaterialEditor",
                "LevelSequence",
				"MovieScene",
				"MovieSceneTracks"
            }
		);
		
		// If you intend to use Editor-Only libraries (like AssetTools) within this module
		// specifically for Editor builds, we can conditionally link them.
		// Ideally, asset importing logic resides in the 'UELTX2Editor' module, 
		// but this safeguard allows mixed usage if necessary.
		if (Target.bBuildEditor)
		{
			PrivateDependencyModuleNames.AddRange(
				new string[]
				{
					"UnrealEd",
					"AssetTools",
                    "MaterialEditor"
                }
			);
		}
	}
}