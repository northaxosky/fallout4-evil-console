#include "Game/ConsoleToggleHook.h"

#include <safetyhook.hpp>

namespace Game::ConsoleToggleHook
{
	namespace
	{
		// Console::ToggleOpenConsole; NG and AE bodies match apart from relocation.
		constexpr REL::ID kToggleOpenConsole{1484235, 2248542, 2248542};

		Handler                g_handler{nullptr};
		safetyhook::InlineHook g_toggle;

		void ToggleOpenConsole()
		{
			if (!g_handler()) {
				g_toggle.call();
			}
		}
	}

	bool Install(Handler a_handler)
	{
		g_handler = a_handler;

		const auto target = kToggleOpenConsole.address();
		auto       hook = safetyhook::InlineHook::create(reinterpret_cast<void*>(target), reinterpret_cast<void*>(&ToggleOpenConsole));
		if (!hook) {
			REX::ERROR("could not hook Console::ToggleOpenConsole at {:#x}", target);
			return false;
		}

		g_toggle = std::move(*hook);
		return true;
	}
}
