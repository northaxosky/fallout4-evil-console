#include "Console/History.h"

namespace Console
{
	History::History(std::size_t a_capacity) :
		capacity_(std::max<std::size_t>(a_capacity, 1))
	{}

	void History::Assign(std::vector<std::string> a_entries)
	{
		entries_.assign(std::make_move_iterator(a_entries.begin()), std::make_move_iterator(a_entries.end()));
		Trim();
		ResetNavigation();
	}

	bool History::Add(std::string a_entry)
	{
		ResetNavigation();
		if (a_entry.find_first_not_of(" \t") == std::string::npos || (!entries_.empty() && entries_.back() == a_entry)) {
			return false;
		}

		entries_.push_back(std::move(a_entry));
		Trim();
		position_ = entries_.size();
		return true;
	}

	std::optional<std::string> History::Previous(std::string_view a_current)
	{
		if (position_ == 0) {
			return std::nullopt;
		}
		if (position_ == entries_.size()) {
			draft_ = a_current;
		}
		return entries_[--position_];
	}

	std::optional<std::string> History::Next()
	{
		if (position_ >= entries_.size()) {
			return std::nullopt;
		}
		return ++position_ == entries_.size() ? draft_ : entries_[position_];
	}

	void History::ResetNavigation() noexcept
	{
		position_ = entries_.size();
		draft_.clear();
	}

	void History::Trim()
	{
		while (entries_.size() > capacity_) {
			entries_.pop_front();
		}
	}
}
