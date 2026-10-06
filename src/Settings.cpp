#include "Settings.h"

namespace Settings
{
	void Load()
	{
		const auto store = REX::FIniSettingStore::GetSingleton();
		store->Init("Data/F4SE/Plugins/Fallout4EvilConsole.ini", "Data/F4SE/Plugins/Fallout4EvilConsoleCustom.ini");
		store->Load();
	}
}
