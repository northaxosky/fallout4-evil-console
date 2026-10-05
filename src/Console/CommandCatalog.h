#pragma once

namespace Console
{
	struct ParameterInfo
	{
		std::string name;
		bool        optional{false};
	};

	struct CommandInfo
	{
		std::string                name;
		std::string                shortName;
		std::string                help;
		std::vector<ParameterInfo> parameters;
		bool                       referenceFunction{false};
	};

	// Immutable command index; lookups ignore ASCII case like the game's compiler.
	class CommandCatalog
	{
	public:
		explicit CommandCatalog(std::vector<CommandInfo> a_commands);

		[[nodiscard]] const CommandInfo* Find(std::string_view a_token) const;

		// Exact matches first, then long-name prefixes, then short-name prefixes, each alphabetical.
		[[nodiscard]] std::vector<const CommandInfo*> Complete(std::string_view a_prefix, std::size_t a_limit) const;

		[[nodiscard]] std::size_t Size() const noexcept { return commands_.size(); }

	private:
		std::vector<CommandInfo> commands_;
	};

	// "Name (Short) Param [Optional]"
	[[nodiscard]] std::string FormatSignature(const CommandInfo& a_command);
}
