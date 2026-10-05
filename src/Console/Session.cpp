#include "Console/Session.h"

#include "Console/CommandExecutor.h"
#include "Console/HistoryStore.h"

namespace Console
{
	Session::Session(CommandExecutor& a_executor, HistoryStore& a_store, std::size_t a_historyCapacity) :
		executor_(a_executor),
		store_(a_store),
		history_(a_historyCapacity)
	{
		history_.Assign(store_.Load(a_historyCapacity));
	}

	void Session::Submit(std::string a_line)
	{
		if (history_.Add(a_line)) {
			store_.Append(a_line);
		}
		executor_.Submit(std::move(a_line));
	}

	void Session::SetCatalog(std::shared_ptr<const CommandCatalog> a_catalog) noexcept
	{
		catalog_.store(std::move(a_catalog), std::memory_order_release);
	}

	std::shared_ptr<const CommandCatalog> Session::GetCatalog() const noexcept
	{
		return catalog_.load(std::memory_order_acquire);
	}
}
