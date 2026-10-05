#pragma once

namespace Console
{
	// Game text is not guaranteed UTF-8; invalid input is reinterpreted as Latin-1.
	[[nodiscard]] std::string ToValidUtf8(std::string_view a_text);
}
