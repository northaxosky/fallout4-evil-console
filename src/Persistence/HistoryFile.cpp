#include "Persistence/HistoryFile.h"

#include <REX/W32/OLE32.h>
#include <REX/W32/SHELL32.h>

namespace Persistence
{
	HistoryFile::HistoryFile(std::filesystem::path a_path) :
		path_(std::move(a_path))
	{}

	std::vector<std::string> HistoryFile::Load(std::size_t a_maximum)
	{
		std::vector<std::string> entries;
		std::ifstream            file{path_, std::ios::binary};
		for (std::string line; std::getline(file, line);) {
			if (line.ends_with('\r')) {
				line.pop_back();
			}
			if (!line.empty()) {
				entries.push_back(std::move(line));
			}
		}

		if (entries.size() > a_maximum) {
			entries.erase(entries.begin(), entries.end() - static_cast<std::ptrdiff_t>(a_maximum));
			std::ofstream compacted{path_, std::ios::binary | std::ios::trunc};
			for (const auto& entry : entries) {
				compacted << entry << '\n';
			}
		}
		return entries;
	}

	void HistoryFile::Append(std::string_view a_entry)
	{
		if (path_.empty()) {
			return;
		}

		std::error_code error;
		std::filesystem::create_directories(path_.parent_path(), error);
		std::ofstream file{path_, std::ios::binary | std::ios::app};
		file << a_entry << '\n';
		if (!file) {
			REX::WARN("could not append console history to {}", path_.string());
		}
	}

	std::optional<std::filesystem::path> DefaultHistoryPath()
	{
		wchar_t*                                                             buffer{nullptr};
		const auto                                                           result = REX::W32::SHGetKnownFolderPath(REX::W32::FOLDERID_Documents, REX::W32::KF_FLAG_DEFAULT, nullptr, std::addressof(buffer));
		const std::unique_ptr<wchar_t[], decltype(&REX::W32::CoTaskMemFree)> documents{buffer, REX::W32::CoTaskMemFree};
		if (result != 0 || !documents) {
			return std::nullopt;
		}

		std::filesystem::path path{documents.get()};
		path /= std::format("My Games/{}/F4SE/{}/History.txt", F4SE::GetSaveFolderName(), F4SE::GetPluginName());
		return path;
	}
}
