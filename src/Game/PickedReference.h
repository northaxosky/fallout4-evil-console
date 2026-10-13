#pragma once

#include "Console/Selection.h"

namespace Game
{
	// Reads the vanilla console pick (Console::GetPickRef) on the game thread.
	class PickedReference final :
		public Console::SelectionSource
	{
	public:
		void Refresh() override;
		void SelectAtCursor() override;

		[[nodiscard]] std::shared_ptr<const Console::Selection> Current() const override;

	private:
		std::atomic<bool>                                      pending_{false};
		std::atomic<std::shared_ptr<const Console::Selection>> current_;
	};
}
