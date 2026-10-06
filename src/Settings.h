#pragma once

namespace Settings
{
	// Off leaves the console key to the vanilla console.
	inline REX::TIniSetting<bool> bReplaceVanillaConsole{"Console", "bReplaceVanillaConsole", true};

	// Reads the packaged INI, then the user's Custom override.
	void Load();
}
