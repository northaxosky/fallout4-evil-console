#pragma once

namespace Console
{
	// Byte range of the command word in a console line, after any "ref." prefix.
	struct CommandToken
	{
		std::size_t begin{0};
		std::size_t end{0};

		[[nodiscard]] std::string_view In(std::string_view a_line) const noexcept { return a_line.substr(begin, end - begin); }
		[[nodiscard]] bool             Contains(std::size_t a_offset) const noexcept { return a_offset >= begin && a_offset <= end; }
	};

	[[nodiscard]] CommandToken FindCommandToken(std::string_view a_line) noexcept;
}
