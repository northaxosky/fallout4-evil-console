#pragma once

#include "Console/CommandExecutor.h"

namespace Game
{
	// Runs lines through the vanilla console submit path for exact vanilla semantics.
	class VanillaCommandExecutor final :
		public Console::CommandExecutor
	{
	public:
		void Submit(std::string a_line) override;
	};
}
