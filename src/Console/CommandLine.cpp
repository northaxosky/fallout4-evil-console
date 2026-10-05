#include "Console/CommandLine.h"

namespace Console
{
	namespace
	{
		[[nodiscard]] bool IsSpace(char a_char) noexcept
		{
			return a_char == ' ' || a_char == '\t';
		}
	}

	CommandToken FindCommandToken(std::string_view a_line) noexcept
	{
		std::size_t begin = 0;
		while (begin < a_line.size() && IsSpace(a_line[begin])) {
			++begin;
		}

		auto end = begin;
		while (end < a_line.size() && !IsSpace(a_line[end])) {
			++end;
		}

		// "player.additem" and "14.disable" call the command on a reference.
		const auto word = a_line.substr(begin, end - begin);
		if (const auto dot = word.rfind('.'); dot != std::string_view::npos) {
			begin += dot + 1;
		}
		return {begin, end};
	}
}
