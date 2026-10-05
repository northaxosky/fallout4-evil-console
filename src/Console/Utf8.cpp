#include "Console/Utf8.h"

namespace Console
{
	namespace
	{
		[[nodiscard]] bool IsContinuation(unsigned char a_byte) noexcept
		{
			return (a_byte & 0xC0u) == 0x80u;
		}

		[[nodiscard]] bool IsValidUtf8(std::string_view a_text) noexcept
		{
			for (std::size_t index = 0; index < a_text.size();) {
				const auto  lead = static_cast<unsigned char>(a_text[index]);
				std::size_t length = lead < 0x80u ? 1 : (lead >> 5) == 0x6u ? 2 :
				                                    (lead >> 4) == 0xEu     ? 3 :
				                                    (lead >> 3) == 0x1Eu    ? 4 :
				                                                              0;
				if (length == 0 || lead == 0xC0u || lead == 0xC1u || lead > 0xF4u || index + length > a_text.size()) {
					return false;
				}
				for (std::size_t offset = 1; offset < length; ++offset) {
					if (!IsContinuation(static_cast<unsigned char>(a_text[index + offset]))) {
						return false;
					}
				}
				// Reject overlong forms, UTF-16 surrogates, and code points above U+10FFFF.
				const auto second = length > 1 ? static_cast<unsigned char>(a_text[index + 1]) : 0u;
				if ((lead == 0xE0u && second < 0xA0u) || (lead == 0xEDu && second > 0x9Fu) ||
					(lead == 0xF0u && second < 0x90u) || (lead == 0xF4u && second > 0x8Fu)) {
					return false;
				}
				index += length;
			}
			return true;
		}
	}

	std::string ToValidUtf8(std::string_view a_text)
	{
		if (IsValidUtf8(a_text)) {
			return std::string{a_text};
		}

		std::string result;
		result.reserve(a_text.size() * 2);
		for (const auto character : a_text) {
			const auto byte = static_cast<unsigned char>(character);
			if (byte < 0x80u) {
				result.push_back(character);
			} else {
				result.push_back(static_cast<char>(0xC0u | (byte >> 6)));
				result.push_back(static_cast<char>(0x80u | (byte & 0x3Fu)));
			}
		}
		return result;
	}
}
