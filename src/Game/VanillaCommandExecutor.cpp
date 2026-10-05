#include "Game/VanillaCommandExecutor.h"

namespace Game
{
	void VanillaCommandExecutor::Submit(std::string a_line)
	{
		const auto tasks = F4SE::GetTaskInterface();
		if (!tasks) {
			REX::ERROR("task interface unavailable; dropped command: {}", a_line);
			return;
		}

		// ExecuteCommand echoes the line, expands ForEachRef[], and compiles against the pick ref; it must run on the game thread.
		tasks->AddTask([line = std::move(a_line)] {
			RE::Console::ExecuteCommand(line.c_str());
		});
	}
}
