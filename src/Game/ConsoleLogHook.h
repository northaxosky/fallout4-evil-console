#pragma once

namespace Console
{
	class OutputLog;
}

namespace Game::ConsoleLogHook
{
	// Mirrors every ConsoleLog::AddString into a_log; the vanilla log still receives the text.
	bool Install(Console::OutputLog& a_log);
}
