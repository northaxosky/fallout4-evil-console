#include "UI/ConsoleView.h"

#include "Console/CommandCatalog.h"
#include "Console/CommandLine.h"
#include "Console/OutputLog.h"
#include "Console/Session.h"
#include "Console/Utf8.h"

namespace UI
{
	namespace
	{
		using EditFlags = dmui::ui::TextEditFlags;
		using EditEvents = dmui::ui::TextEditEvents;

		constexpr std::size_t kMaxOutputLines = 4000;
		constexpr std::size_t kMaxSuggestions = 12;
		constexpr char        kConsoleKeyCharacter = '`';

		// The input sits at the bottom of the console, so popups open upward from the caret line.
		constexpr dmui::ui::Vec2 kAboveCaret{0.0f, 1.0f};
	}

	ConsoleView::ConsoleView(dmui::Client& a_client, Console::Session& a_session, const Console::OutputLog& a_output) :
		client_(a_client),
		session_(a_session),
		output_(a_output)
	{}

	void ConsoleView::OnOpened() noexcept
	{
		pendingFlags_ = pendingFlags_ | EditFlags::kRequestFocus;
		session_.GetHistory().ResetNavigation();
		ClosePopup();
	}

	void ConsoleView::Draw(float a_available)
	{
		// Suggestions point into the catalog, so a replaced catalog invalidates them.
		if (auto catalog = session_.GetCatalog(); catalog != catalog_) {
			ClosePopup();
			catalog_ = std::move(catalog);
		}

		float inputHeight{};
		{
			const dmui::FontGuard monospace{client_, DMUI_FONT_ROLE_MONOSPACE};
			inputHeight = dmui::ui::GetFrameHeight();
		}
		DMUI_StyleMetrics metrics{};
		(void)dmui::ui::GetStyleMetrics(metrics);

		PullOutput();
		DrawOutput(a_available - inputHeight - metrics.itemSpacing.y);
		DrawInput();
	}

	void ConsoleView::PullOutput()
	{
		incoming_.clear();
		output_.ReadFrom(outputNext_, incoming_);
		if (incoming_.empty()) {
			return;
		}

		for (const auto& line : incoming_) {
			if (std::exchange(hasOutput_, true)) {
				outputText_.push_back('\n');
				lineOffsets_.push_back(outputText_.size());
			}
			outputText_ += Console::ToValidUtf8(line);
		}

		// Drop the older half at once so trimming stays amortized.
		if (lineOffsets_.size() > kMaxOutputLines) {
			const auto keepFrom = lineOffsets_[lineOffsets_.size() - kMaxOutputLines / 2];
			outputText_.erase(0, keepFrom);
			std::erase_if(lineOffsets_, [&](std::size_t a_offset) { return a_offset < keepFrom; });
			for (auto& offset : lineOffsets_) {
				offset -= keepFrom;
			}
		}

		++outputRevision_;
	}

	void ConsoleView::DrawOutput(float a_height)
	{
		const dmui::TextViewRequest request{
			.text = outputText_,
			.lineOffsets = lineOffsets_,
			.contentRevision = outputRevision_,
			.viewport = {0.0f, std::max(a_height, 1.0f)}};

		// New output always scrolls to the newest line.
		if (outputState_.contentRevision != outputRevision_) {
			(void)dmui::RevealTextOffset(request, outputState_, lineOffsets_.back());
		}
		(void)client_.DrawTextView("##output", request, outputState_);
	}

	void ConsoleView::DrawInput()
	{
		dmui::FontGuard monospace{client_, DMUI_FONT_ROLE_MONOSPACE};

		const auto flags = EditFlags::kHistoryKeys | EditFlags::kCompletionKey | EditFlags::kKeepFocusOnSubmit |
		                   EditFlags::kCaptureCancel | std::exchange(pendingFlags_, EditFlags::kNone);
		dmui::ui::SetNextItemWidth(-1.0f);
		dmui::ui::TextEditState state{};
		const auto              edited = dmui::ui::InputTextEditor("##command", "Enter a command", input_, flags, pendingCursor_, state);
		if (state.active) {
			cursor_ = state.cursor;
		}

		const auto has = [&](EditEvents a_event) { return dmui::ui::HasTextEditEvent(state, a_event); };

		if (has(EditEvents::kSubmitted)) {
			if (!input_.empty()) {
				session_.Submit(input_);
			}
			ReplaceInput({}, 0);
			ClosePopup();
			return;
		}

		// Focus blocks the game's console key, so its character arrives as text instead.
		if (edited && input_.contains(kConsoleKeyCharacter)) {
			const auto before = std::ranges::count(std::string_view{input_}.substr(0, std::min(cursor_, input_.size())), kConsoleKeyCharacter);
			std::erase(input_, kConsoleKeyCharacter);
			ReplaceInput(std::move(input_), cursor_ - static_cast<std::size_t>(before));
			ClosePopup();
			closeRequested_ = true;
			return;
		}

		if (edited) {
			session_.GetHistory().ResetNavigation();
			RefreshSuggestions(cursor_, false);
		}

		if (has(EditEvents::kCompletion)) {
			if (popupOpen_) {
				AcceptSuggestion();
			} else {
				RefreshSuggestions(cursor_, true);
			}
		}

		const auto previous = has(EditEvents::kHistoryPrevious);
		if (previous || has(EditEvents::kHistoryNext)) {
			if (popupOpen_) {
				const auto count = suggestions_.size();
				// The list grows upward from the input, so Up moves away from it.
				selected_ = (selected_ + (previous ? 1 : count - 1)) % count;
			} else {
				auto& history = session_.GetHistory();
				if (auto entry = previous ? history.Previous(input_) : history.Next()) {
					const auto length = entry->size();
					ReplaceInput(std::move(*entry), length);
				}
			}
		}

		if (has(EditEvents::kCanceled)) {
			if (popupOpen_) {
				ClosePopup();
			} else {
				closeRequested_ = true;
			}
		}

		if (popupOpen_) {
			DrawPopup(state);
		} else {
			DrawHint(state);
		}
	}

	void ConsoleView::DrawPopup(const dmui::ui::TextEditState& a_state)
	{
		if (!a_state.active || suggestions_.empty()) {
			return;
		}

		if (!dmui::ui::BeginTooltipAt(a_state.caretPosition, kAboveCaret)) {
			return;
		}

		const auto* selected = suggestions_[selected_];
		dmui::ui::TextUnformatted(Console::FormatSignature(*selected));
		if (!selected->help.empty()) {
			dmui::ui::TextDisabled("%s", selected->help.c_str());
		}
		dmui::ui::Separator();

		// Best match sits next to the input line.
		for (auto index = suggestions_.size(); index-- > 0;) {
			const auto* command = suggestions_[index];
			const auto  label = command->shortName.empty() ? command->name : std::format("{}  ({})", command->name, command->shortName);
			(void)dmui::ui::Selectable(label.c_str(), index == selected_);
		}
		dmui::ui::EndTooltip();
	}

	void ConsoleView::DrawHint(const dmui::ui::TextEditState& a_state)
	{
		if (!a_state.active || !catalog_) {
			return;
		}

		const auto  token = Console::FindCommandToken(input_);
		const auto* command = catalog_->Find(token.In(input_));
		if (!command || a_state.cursor <= token.end) {
			return;
		}

		if (!dmui::ui::BeginTooltipAt(a_state.caretPosition, kAboveCaret)) {
			return;
		}
		dmui::ui::TextUnformatted(Console::FormatSignature(*command));
		if (!command->help.empty()) {
			dmui::ui::TextDisabled("%s", command->help.c_str());
		}
		dmui::ui::EndTooltip();
	}

	void ConsoleView::ReplaceInput(std::string a_text, std::size_t a_cursor)
	{
		// Applied by the editor next frame, before that frame's typing.
		input_ = std::move(a_text);
		pendingCursor_ = std::min(a_cursor, input_.size());
		pendingFlags_ = pendingFlags_ | EditFlags::kReload;
	}

	void ConsoleView::RefreshSuggestions(std::size_t a_cursor, bool a_explicit)
	{
		ClosePopup();
		if (!catalog_) {
			return;
		}

		const auto token = Console::FindCommandToken(input_);
		if (token.begin == token.end || !token.Contains(a_cursor)) {
			return;
		}

		suggestions_ = catalog_->Complete(token.In(input_), kMaxSuggestions);
		if (suggestions_.empty()) {
			return;
		}

		// Tab with a single candidate completes immediately.
		if (a_explicit && suggestions_.size() == 1) {
			AcceptSuggestion();
			return;
		}
		popupOpen_ = true;
	}

	void ConsoleView::AcceptSuggestion()
	{
		if (suggestions_.empty()) {
			ClosePopup();
			return;
		}

		const auto  token = Console::FindCommandToken(input_);
		const auto& name = suggestions_[selected_]->name;
		auto        text = input_.substr(0, token.begin) + name;
		auto        rest = std::string_view{input_}.substr(token.end);
		if (rest.empty()) {
			text.push_back(' ');
		}
		const auto cursor = text.size();
		text += rest;
		ClosePopup();
		ReplaceInput(std::move(text), cursor);
	}

	void ConsoleView::ClosePopup() noexcept
	{
		suggestions_.clear();
		selected_ = 0;
		popupOpen_ = false;
	}
}
