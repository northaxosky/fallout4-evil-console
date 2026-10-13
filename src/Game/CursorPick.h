#pragma once

namespace Game::CursorPick
{
	// Lets vanilla console picking use the mouse cursor while our console replaces the vanilla menu.
	bool Install();

	// Game thread only. Selects the next reference under the cursor, as clicking with the vanilla console open.
	void PickNextAtCursor();
}
