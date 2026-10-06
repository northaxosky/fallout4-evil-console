#include "UI/Frontend.h"

#include "UI/ConsoleView.h"

namespace UI::Frontend
{
	namespace
	{
		constexpr auto kClientId = "dearmodding.evil-console";
		constexpr auto kClientDisplayName = "Evil Console";
		constexpr auto kClientIcon = "terminal-window";
		constexpr auto kRequiredAbiMinor = 1u;

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
		std::atomic<bool>             g_ready{false};
		std::atomic<bool>             g_togglePending{false};

		// The game still sees the key press that closed the console once focus is released.
		constexpr auto                        kConsoleKeyEcho = std::chrono::milliseconds{500};
		std::chrono::steady_clock::time_point g_consoleKeyClosed{};

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
			if (const auto request = g_view->TakeCloseRequest(); request != CloseRequest::kNone) {
				if (request == CloseRequest::kConsoleKey) {
					g_consoleKeyClosed = std::chrono::steady_clock::now();
				}
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

		// The input editor and focused overlays arrived in ABI 2.1.
		if (g_client->AbiMinor() < kRequiredAbiMinor) {
			REX::ERROR("DearModdingUI ABI 2.{} is too old; update DearModdingUI to 0.2.1 or newer", g_client->AbiMinor());
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

		// Toggles arrive from the game thread; apply them on the render thread.
		const auto observer = g_client->AddFrameObserver([] {
			if (!g_togglePending.exchange(false)) {
				return;
			}
			if (!g_open && std::chrono::steady_clock::now() - g_consoleKeyClosed < kConsoleKeyEcho) {
				REX::DEBUG("ignored the console key press that closed the console");
				return;
			}
			g_open ? Close() : Open();
		});
		if (!observer) {
			REX::ERROR("could not register the console frame observer, result: {}", g_client->LastResult());
			return false;
		}

		g_ready = true;
		REX::INFO("registered '{}' with DearModdingUI", kClientId);
		return true;
	}

	bool RequestToggle() noexcept
	{
		if (!g_ready) {
			return false;
		}
		g_togglePending = true;
		return true;
	}
}
