#include "Console/OutputLog.h"
#include "Console/Session.h"
#include "Game/CommandTable.h"
#include "Game/ConsoleLogHook.h"
#include "Game/VanillaCommandExecutor.h"
#include "Persistence/HistoryFile.h"
#include "UI/Frontend.h"

namespace Main
{
	namespace
	{
		constexpr std::size_t kOutputCapacity = 4096;
		constexpr std::size_t kHistoryCapacity = 500;

		Console::OutputLog                        g_output{kOutputCapacity};
		Game::VanillaCommandExecutor              g_executor;
		std::unique_ptr<Persistence::HistoryFile> g_historyFile;
		std::unique_ptr<Console::Session>         g_session;

		void OnMessage(F4SE::MessagingInterface::Message* a_message)
		{
			switch (a_message->type) {
				case F4SE::MessagingInterface::kPostPostLoad:
					(void)UI::Frontend::Connect(*g_session, g_output);
					break;
				case F4SE::MessagingInterface::kGameDataReady:
					// Snapshot after all plugins have patched the function tables.
					g_session->SetCatalog(Game::CommandTable::BuildCatalog());
					break;
				default:
					break;
			}
		}
	}

	bool InitPlugin(const F4SE::LoadInterface* a_f4se)
	{
		static std::once_flag once;
		static bool           initialized = false;
		std::call_once(once, [&]() {
			F4SE::Init(a_f4se);

			// Installed at load so output printed before the first open is still captured.
			if (!Game::ConsoleLogHook::Install(g_output)) {
				return;
			}

			auto historyPath = Persistence::DefaultHistoryPath();
			if (!historyPath) {
				REX::WARN("could not resolve the Documents folder; console history will not persist");
			}
			g_historyFile = std::make_unique<Persistence::HistoryFile>(historyPath.value_or(std::filesystem::path{}));
			g_session = std::make_unique<Console::Session>(g_executor, *g_historyFile, kHistoryCapacity);

			const auto messaging = F4SE::GetMessagingInterface();
			if (!messaging || !messaging->RegisterListener(OnMessage)) {
				REX::ERROR("could not register for F4SE messages");
				return;
			}

			REX::INFO("Loaded");

			initialized = true;
		});

		return initialized;
	}

	F4SE_PLUGIN_QUERY(const F4SE::QueryInterface*, F4SE::PluginInfo* a_info)
	{
		if (const auto data = F4SE::PluginVersionData::GetSingleton()) {
			a_info->infoVersion = F4SE::PluginInfo::kVersion;
			a_info->name = data->GetPluginName().data();
			a_info->version = data->GetPluginVersion().pack();
		}

		return true;
	}

	F4SE_PLUGIN_LOAD(const F4SE::LoadInterface* a_f4se)
	{
		// OG does not support PreLoading
		return InitPlugin(a_f4se);
	}

	F4SE_PLUGIN_PRELOAD(const F4SE::LoadInterface* a_f4se)
	{
		return InitPlugin(a_f4se);
	}
}
