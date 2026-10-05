#include "Game/CommandTable.h"

#include "Console/CommandCatalog.h"

namespace Game::CommandTable
{
	namespace
	{
		[[nodiscard]] std::string Copy(const char* a_text)
		{
			return a_text ? a_text : "";
		}

		void Append(std::span<RE::SCRIPT_FUNCTION> a_functions, std::vector<Console::CommandInfo>& a_out)
		{
			for (const auto& function : a_functions) {
				Console::CommandInfo command{
					.name = Copy(function.functionName),
					.shortName = Copy(function.shortName),
					.help = Copy(function.helpString),
					.parameters = {},
					.referenceFunction = function.referenceFunction};
				if (function.parameters) {
					for (std::uint16_t index = 0; index < function.paramCount; ++index) {
						const auto& parameter = function.parameters[index];
						command.parameters.push_back({.name = Copy(parameter.paramName), .optional = parameter.optional});
					}
				}
				a_out.push_back(std::move(command));
			}
		}
	}

	std::shared_ptr<const Console::CommandCatalog> BuildCatalog()
	{
		std::vector<Console::CommandInfo> commands;
		Append(RE::SCRIPT_FUNCTION::GetConsoleFunctions(), commands);
		Append(RE::SCRIPT_FUNCTION::GetScriptFunctions(), commands);
		return std::make_shared<const Console::CommandCatalog>(std::move(commands));
	}
}
