#pragma once

namespace Console
{
	class OutputLog;
	class Session;
}

namespace UI::Frontend
{
	// Registers the console overlay with the DearModdingUI host; call at kPostPostLoad.
	bool Connect(Console::Session& a_session, const Console::OutputLog& a_output);

	// Any thread; false until the console is registered, so callers can fall back to vanilla.
	bool RequestToggle() noexcept;
}
