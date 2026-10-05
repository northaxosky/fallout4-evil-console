#pragma once

namespace Console
{
	// Runs submitted console lines; the frontend never talks to the game directly.
	class CommandExecutor
	{
	public:
		virtual ~CommandExecutor() = default;

		// Callable from any thread; execution may be deferred.
		virtual void Submit(std::string a_line) = 0;
	};
}
