#include "Game/ConsoleLogHook.h"

#include "Console/OutputLog.h"

#include <safetyhook.hpp>

namespace Game::ConsoleLogHook
{
	namespace
	{
		Console::OutputLog*    g_log{nullptr};
		safetyhook::InlineHook g_addString;

		// AddString is the single sink behind Print/PrintLine and is entered from any thread.
		void AddString(RE::ConsoleLog* a_this, const char* a_text)
		{
			if (a_text) {
				g_log->Append(a_text);
			}
			g_addString.call(a_this, a_text);
		}
	}

	bool Install(Console::OutputLog& a_log)
	{
		g_log = std::addressof(a_log);

		const REL::Relocation<std::uintptr_t> target{RE::ID::ConsoleLog::AddString};
		auto                                  hook = safetyhook::InlineHook::create(reinterpret_cast<void*>(target.address()), reinterpret_cast<void*>(&AddString));
		if (!hook) {
			REX::ERROR("could not hook ConsoleLog::AddString at {:#x}", target.address());
			return false;
		}

		g_addString = std::move(*hook);
		return true;
	}
}
