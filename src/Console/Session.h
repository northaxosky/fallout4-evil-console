#pragma once

#include "Console/CommandCatalog.h"
#include "Console/History.h"

namespace Console
{
	class CommandExecutor;
	class HistoryStore;

	// Frontend-facing console state: submission, history, and the command catalog.
	class Session
	{
	public:
		Session(CommandExecutor& a_executor, HistoryStore& a_store, std::size_t a_historyCapacity);

		void Submit(std::string a_line);

		[[nodiscard]] History& GetHistory() noexcept { return history_; }

		// The catalog is built on the game thread and read by the frontend.
		void                                                SetCatalog(std::shared_ptr<const CommandCatalog> a_catalog) noexcept;
		[[nodiscard]] std::shared_ptr<const CommandCatalog> GetCatalog() const noexcept;

	private:
		CommandExecutor&                                   executor_;
		HistoryStore&                                      store_;
		History                                            history_;
		std::atomic<std::shared_ptr<const CommandCatalog>> catalog_;
	};
}
