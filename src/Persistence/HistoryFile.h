#pragma once

#include "Console/HistoryStore.h"

namespace Persistence
{
	// One UTF-8 line per entry, appended on submit and compacted on load.
	class HistoryFile final :
		public Console::HistoryStore
	{
	public:
		explicit HistoryFile(std::filesystem::path a_path);

		[[nodiscard]] std::vector<std::string> Load(std::size_t a_maximum) override;
		void                                   Append(std::string_view a_entry) override;

	private:
		std::filesystem::path path_;
	};

	// Documents\My Games\<save folder>\F4SE\<plugin>\History.txt, beside the F4SE log.
	[[nodiscard]] std::optional<std::filesystem::path> DefaultHistoryPath();
}
