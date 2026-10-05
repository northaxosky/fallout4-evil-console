#pragma once

namespace Console
{
	class CommandCatalog;
}

namespace Game::CommandTable
{
	// Snapshots the live console and script function tables, including entries other plugins added.
	[[nodiscard]] std::shared_ptr<const Console::CommandCatalog> BuildCatalog();
}
