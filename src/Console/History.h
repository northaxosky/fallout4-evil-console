#pragma once

namespace Console
{
	// Bounded submitted-line history with shell-style Up/Down navigation.
	class History
	{
	public:
		explicit History(std::size_t a_capacity);

		void Assign(std::vector<std::string> a_entries);

		// Returns false when a_entry is blank or repeats the newest entry.
		bool Add(std::string a_entry);

		// a_current is kept as the draft restored after stepping past the newest entry.
		[[nodiscard]] std::optional<std::string> Previous(std::string_view a_current);
		[[nodiscard]] std::optional<std::string> Next();

		void ResetNavigation() noexcept;

	private:
		void Trim();

		std::deque<std::string> entries_;
		std::string             draft_;
		std::size_t             position_{0};
		std::size_t             capacity_;
	};
}
