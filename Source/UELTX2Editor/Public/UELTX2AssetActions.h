#pragma once

#include "CoreMinimal.h"
#include "ToolMenus.h"

/**
 * Manages Content Browser context menu extensions.
 * Adds the "Generative AI -> Animate with LTX-2" action to Textures.
 */
class FUELTX2AssetActions
{
public:
	/** Call this during Module Startup to bind the context menus */
	static void Register();

	/** Call this during Module Shutdown to clean up */
	static void Unregister();

private:
	/** 
	 * The actual logic executed when the user clicks the menu item.
	 * Identifies selected textures and passes them to the Runtime Subsystem.
	 */
	static void ExecuteAnimateTexture(const FToolMenuContext& MenuContext);
};