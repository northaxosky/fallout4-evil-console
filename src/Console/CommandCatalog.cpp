#include "Console/CommandCatalog.h"

namespace Console
{
	namespace
	{
		[[nodiscard]] char Fold(char a_char) noexcept
		{
			return a_char >= 'A' && a_char <= 'Z' ? static_cast<char>(a_char - 'A' + 'a') : a_char;
		}

		[[nodiscard]] bool EqualsFolded(std::string_view a_left, std::string_view a_right) noexcept
		{
			return std::ranges::equal(a_left, a_right, {}, Fold, Fold);
		}

		[[nodiscard]] bool StartsWithFolded(std::string_view a_text, std::string_view a_prefix) noexcept
		{
			return a_text.size() >= a_prefix.size() && EqualsFolded(a_text.substr(0, a_prefix.size()), a_prefix);
		}

		[[nodiscard]] bool LessFolded(std::string_view a_left, std::string_view a_right) noexcept
		{
			return std::ranges::lexicographical_compare(a_left, a_right, {}, Fold, Fold);
		}
	}

	CommandCatalog::CommandCatalog(std::vector<CommandInfo> a_commands) :
		commands_(std::move(a_commands))
	{
		std::erase_if(commands_, [](const CommandInfo& a_command) { return a_command.name.empty(); });
		std::ranges::stable_sort(commands_, LessFolded, &CommandInfo::name);
		// Console and script tables overlap; the earlier entry wins, matching the compiler's lookup order.
		const auto duplicates = std::ranges::unique(commands_, EqualsFolded, &CommandInfo::name);
		commands_.erase(duplicates.begin(), duplicates.end());
	}

	const CommandInfo* CommandCatalog::Find(std::string_view a_token) const
	{
		if (a_token.empty()) {
			return nullptr;
		}

		for (const auto& command : commands_) {
			if (EqualsFolded(command.name, a_token)) {
				return std::addressof(command);
			}
		}
		for (const auto& command : commands_) {
			if (EqualsFolded(command.shortName, a_token)) {
				return std::addressof(command);
			}
		}
		return nullptr;
	}

	std::vector<const CommandInfo*> CommandCatalog::Complete(std::string_view a_prefix, std::size_t a_limit) const
	{
		std::vector<const CommandInfo*> exact;
		std::vector<const CommandInfo*> byName;
		std::vector<const CommandInfo*> byShortName;
		if (a_prefix.empty()) {
			return exact;
		}

		for (const auto& command : commands_) {
			if (EqualsFolded(command.name, a_prefix) || EqualsFolded(command.shortName, a_prefix)) {
				exact.push_back(std::addressof(command));
			} else if (StartsWithFolded(command.name, a_prefix)) {
				byName.push_back(std::addressof(command));
			} else if (StartsWithFolded(command.shortName, a_prefix)) {
				byShortName.push_back(std::addressof(command));
			}
		}

		exact.append_range(byName);
		exact.append_range(byShortName);
		if (exact.size() > a_limit) {
			exact.resize(a_limit);
		}
		return exact;
	}

	std::string FormatSignature(const CommandInfo& a_command)
	{
		auto signature = a_command.name;
		if (!a_command.shortName.empty() && !EqualsFolded(a_command.shortName, a_command.name)) {
			signature += std::format(" ({})", a_command.shortName);
		}
		for (const auto& parameter : a_command.parameters) {
			signature += parameter.optional ? std::format(" [{}]", parameter.name) : std::format(" {}", parameter.name);
		}
		return signature;
	}
}
