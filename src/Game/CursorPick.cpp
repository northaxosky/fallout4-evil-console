#include "Game/CursorPick.h"

#include <REL/THook.h>

namespace Game::CursorPick
{
	namespace
	{
		// Console::CalculateConsolePickRefs reads MenuCursor only when it sees the Console menu open,
		// and otherwise picks at the screen center. Its only use of that check is the cursor choice.
		constexpr REL::ID kCalculateConsolePickRefs{46678, 2248549, 2248549};
		constexpr REL::ID kPickNextRef{1404232, 2248541, 2248541};

		// Offset of the call to UI::GetMenuOpen inside CalculateConsolePickRefs.
		[[nodiscard]] std::ptrdiff_t MenuOpenCallOffset()
		{
			return REX::FModule::IsRuntimeOG() ? 0x8B : 0x94;
		}

		using MenuOpenFn = bool(RE::UI*, const RE::BSFixedString&);

		std::unique_ptr<REL::THook<MenuOpenFn>> g_menuOpen;
		bool                                    g_atCursor{false};

		bool IsConsoleOpenForPick(RE::UI* a_ui, const RE::BSFixedString& a_menu)
		{
			return g_atCursor || (*g_menuOpen)(a_ui, a_menu);
		}
	}

	bool Install()
	{
		g_menuOpen = std::make_unique<REL::THook<MenuOpenFn>>(kCalculateConsolePickRefs, MenuOpenCallOffset(), &IsConsoleOpenForPick);
		if (!g_menuOpen->Init() || !g_menuOpen->Enable()) {
			REX::ERROR("could not hook the console pick cursor check");
			g_menuOpen.reset();
			return false;
		}
		return true;
	}

	void PickNextAtCursor()
	{
		if (!g_menuOpen) {
			return;
		}

		using PickNextRefFn = RE::ObjectRefHandle*(RE::ObjectRefHandle*, bool);
		static const REL::Relocation<PickNextRefFn*> pickNextRef{kPickNextRef};

		RE::ObjectRefHandle picked;
		g_atCursor = true;
		pickNextRef(std::addressof(picked), false);
		g_atCursor = false;
	}
}
