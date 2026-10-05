#pragma once

namespace Console
{
	// Durable backing for History, kept out of the logic layer.
	class HistoryStore
	{
	public:
		virtual ~HistoryStore() = default;

		[[nodiscard]] virtual std::vector<std::string> Load(std::size_t a_maximum) = 0;
		virtual void                                   Append(std::string_view a_entry) = 0;
	};
}
