#pragma once

#include <DearModdingUI/Client.h>
#include <DearModdingUI/TextView.h>
#include <DearModdingUI/UI.h>

namespace Console
{
	class CommandCatalog;
	struct CommandInfo;
	class OutputLog;
	class Session;
}

namespace UI
{
	// Console window contents: output pane, command input, completion popup, and signature hint.
	class ConsoleView
	{
	public:
		ConsoleView(dmui::Client& a_client, Console::Session& a_session, const Console::OutputLog& a_output);

		void OnOpened() noexcept;

		// a_available is the content height left in the window for the output pane and input line.
		void Draw(float a_available);

		// Escape with nothing left to dismiss asks the frontend to close the console.
		[[nodiscard]] bool TakeCloseRequest() noexcept { return std::exchange(closeRequested_, false); }

	private:
		void PullOutput();
		void DrawOutput(float a_height);
		void DrawInput();
		void DrawPopup(const dmui::ui::TextEditState& a_state);
		void DrawHint(const dmui::ui::TextEditState& a_state);

		void ReplaceInput(std::string a_text, std::size_t a_cursor);
		void RefreshSuggestions(std::size_t a_cursor, bool a_explicit);
		void AcceptSuggestion();
		void ClosePopup() noexcept;

		dmui::Client&             client_;
		Console::Session&         session_;
		const Console::OutputLog& output_;

		std::string              outputText_;
		std::vector<std::size_t> lineOffsets_{0};
		std::uint64_t            outputRevision_{1};
		std::uint64_t            outputNext_{0};
		dmui::TextViewState      outputState_;
		std::vector<std::string> incoming_;
		bool                     hasOutput_{false};

		std::string             input_;
		dmui::ui::TextEditFlags pendingFlags_{dmui::ui::TextEditFlags::kNone};
		std::size_t             pendingCursor_{0};
		std::size_t             cursor_{0};

		std::shared_ptr<const Console::CommandCatalog> catalog_;
		std::vector<const Console::CommandInfo*>       suggestions_;
		std::size_t                                    selected_{0};
		bool                                           popupOpen_{false};
		bool                                           closeRequested_{false};
	};
}
