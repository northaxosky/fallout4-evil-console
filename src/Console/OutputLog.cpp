#include "Console/OutputLog.h"

namespace Console
{
	OutputLog::OutputLog(std::size_t a_capacity) :
		capacity_(std::max<std::size_t>(a_capacity, 1))
	{}

	void OutputLog::Append(std::string_view a_text)
	{
		const std::scoped_lock guard{lock_};
		for (auto newline = a_text.find('\n'); newline != std::string_view::npos; newline = a_text.find('\n')) {
			auto line = a_text.substr(0, newline);
			if (line.ends_with('\r')) {
				line.remove_suffix(1);
			}
			pending_.append(line);
			PushLine(std::exchange(pending_, {}));
			a_text.remove_prefix(newline + 1);
		}
		pending_.append(a_text);
	}

	void OutputLog::ReadFrom(std::uint64_t& a_next, std::vector<std::string>& a_out) const
	{
		const std::scoped_lock guard{lock_};
		a_next = std::max(a_next, firstSequence_);
		const auto start = static_cast<std::size_t>(a_next - firstSequence_);
		a_out.insert(a_out.end(), lines_.begin() + static_cast<std::ptrdiff_t>(std::min(start, lines_.size())), lines_.end());
		a_next = firstSequence_ + lines_.size();
	}

	void OutputLog::PushLine(std::string a_line)
	{
		lines_.push_back(std::move(a_line));
		if (lines_.size() > capacity_) {
			lines_.pop_front();
			++firstSequence_;
		}
	}
}
