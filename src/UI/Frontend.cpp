#include "UI/Frontend.h"

#include "UI/ConsoleView.h"

namespace UI::Frontend
{
	namespace
	{
		constexpr auto kClientId = "dearmodding.evil-console";
		constexpr auto kClientDisplayName = "Evil Console";
		constexpr auto kClientIcon = "terminal-window";
		constexpr auto kToggleChord = "Grave";

		constexpr DMUI_ManagedOverlayOptions kOverlayDefaults{
			.anchor = DMUI_OVERLAY_ANCHOR_FREE,
			.offset = {32.0f, 32.0f},
			.size = {960.0f, 440.0f},
			.minimumSize = {480.0f, 200.0f},
			.opacity = 0.94f,
			.contentScale = 1.0f,
			.backgroundVisible = 1,
			.borderVisible = 1,
			.allowArrangement = 1};

		// The host retains callbacks into these, so they live for the whole process.
		std::unique_ptr<dmui::Client> g_client;
		std::unique_ptr<ConsoleView>  g_view;
		DMUI_PageHandle               g_page{DMUI_INVALID_PAGE_HANDLE};
		bool                          g_open{false};
		std::uint64_t                 g_focusGeneration{0};

		void Close()
		{
			if (!std::exchange(g_open, false)) {
				return;
			}
			// Dropping the last frame demand also ends focus.
			if (!g_client->ReleaseFrame(g_page)) {
				REX::WARN("could not hide the console overlay, result: {}", g_client->LastResult());
			}
		}

		// The console is visible only while it holds input focus.
		void Open()
		{
			if (g_open) {
				return;
			}
			if (!g_client->RequestFrame(g_page)) {
				REX::WARN("could not show the console overlay, result: {}", g_client->LastResult());
				return;
			}
			if (!g_client->RequestOverlayFocus(g_page)) {
				REX::WARN("could not focus the console overlay, result: {}", g_client->LastResult());
				(void)g_client->ReleaseFrame(g_page);
				return;
			}

			const auto focus = g_client->QueryOverlayFocus(g_page);
			g_focusGeneration = focus ? focus->generation : 0;
			g_open = true;
			g_view->OnOpened();
		}

		// The host ends focus on loads, when the shell opens, or on failure; follow it closed.
		[[nodiscard]] bool LostFocus()
		{
			const auto focus = g_client->QueryOverlayFocus(g_page);
			if (!focus || focus->focused || focus->generation != g_focusGeneration) {
				return false;
			}
			REX::DEBUG("console focus ended, reason: {}", focus->endReason);
			return true;
		}

		// Content height below the cursor, excluding the window's bottom padding.
		[[nodiscard]] float AvailableHeight()
		{
			const auto        placement = g_client->QueryOverlay(g_page);
			DMUI_StyleMetrics metrics{};
			if (!placement || dmui::ui::GetStyleMetrics(metrics) != DMUI_RESULT_OK) {
				return 0.0f;
			}
			return placement->size.y - dmui::ui::GetCursorPosY() - metrics.windowPadding.y;
		}

		void DrawOverlay()
		{
			if (LostFocus()) {
				Close();
				return;
			}
			g_view->Draw(AvailableHeight());
			if (g_view->TakeCloseRequest()) {
				Close();
			}
		}
	}

	bool Connect(Console::Session& a_session, const Console::OutputLog& a_output)
	{
		const auto version = F4SE::GetPluginVersion();
		g_client = std::make_unique<dmui::Client>(
			kClientId,
			kClientDisplayName,
			dmui::Version{static_cast<std::uint16_t>(version.major()), static_cast<std::uint16_t>(version.minor())},
			kClientIcon);

		if (!g_client->Connect()) {
			if (g_client->HostPresent()) {
				REX::ERROR("DearModdingUI registration failed, result: {}", g_client->LastResult());
			} else {
				REX::WARN("no DearModdingUI host is loaded; the console is unavailable this session");
			}
			return false;
		}

		g_view = std::make_unique<ConsoleView>(*g_client, a_session, a_output);

		const auto page = g_client->AddPage(
			{.id = "console",
				.displayName = kClientDisplayName,
				.summary = "Console output and command input.",
				.kind = DMUI_PAGE_KIND_OVERLAY,
				.iconName = kClientIcon},
			DrawOverlay);
		if (!page) {
			REX::ERROR("could not register the console overlay, result: {}", g_client->LastResult());
			return false;
		}
		g_page = *page;

		if (!g_client->ConfigureOverlay(g_page, kOverlayDefaults)) {
			REX::WARN("could not configure the console overlay, result: {}", g_client->LastResult());
		}

		const auto toggle = g_client->AddHotkeyAction(
			"toggle",
			"Toggle console",
			kToggleChord,
			[](bool a_pressed) {
				if (a_pressed) {
					g_open ? Close() : Open();
				}
			});
		if (!toggle) {
			REX::WARN("could not register the console hotkey, result: {}", g_client->LastResult());
		}

		REX::INFO("registered '{}' with DearModdingUI", kClientId);
		return true;
	}
}
