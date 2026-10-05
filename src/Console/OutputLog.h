#pragma once

namespace Console
{
	// Bounded, thread-safe store of complete console output lines.
	// Readers track their own sequence cursor, so any number of views can consume it.
	class OutputLog
	{
	public:
		explicit OutputLog(std::size_t a_capacity);

		// Accepts arbitrary fragments; text after the last newline waits for the next append.
		void Append(std::string_view a_text);

		// Appends lines at or after a_next and advances it; evicted lines are skipped.
		void ReadFrom(std::uint64_t& a_next, std::vector<std::string>& a_out) const;

	private:
		void PushLine(std::string a_line);

		mutable std::mutex      lock_;
		std::deque<std::string> lines_;
		std::string             pending_;
		std::uint64_t           firstSequence_{0};
		std::size_t             capacity_;
	};
}
