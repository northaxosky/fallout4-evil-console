#pragma once

namespace Game::ConsoleToggleHook
{
	// Returns true when the press was handled; false falls through to vanilla.
	using Handler = bool (*)();

	// Redirects the console key's toggle; startup console creation is untouched.
	bool Install(Handler a_handler);
}
