#pragma once

#include "CoreMinimal.h"
#include "Styling/ISlateStyle.h"

/**
 * Singleton class to manage the Slate Style Set for UELTX2.
 * Used to load and access icons (like the Toolbar button image).
 */
class FUELTX2Style
{
public:
	static void Initialize();
	static void Shutdown();

	/** Reloads textures used by slate renderer */
	static void ReloadTextures();

	/** @return The Slate style set for the plugin */
	static const ISlateStyle& Get();

	static FName GetStyleSetName();

private:
	static TSharedRef< class FSlateStyleSet > Create();

	static TSharedPtr< class FSlateStyleSet > StyleInstance;
};