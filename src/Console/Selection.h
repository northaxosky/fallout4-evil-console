#pragma once

namespace Console
{
	// Game-independent description of a form for display.
	struct FormSummary
	{
		std::uint32_t formID{0};
		std::string   type;
		std::string   name;
		std::string   editorID;
		std::string   definedIn;
		std::string   lastChangedBy;
	};

	struct ActorSummary
	{
		std::int32_t level{0};
		std::string  race;
		float        health{0.0f};
		float        healthMax{0.0f};
	};

	// Snapshot of the console's selected reference, taken on the game thread.
	struct Selection
	{
		FormSummary                 reference;
		FormSummary                 base;
		std::array<float, 3>        position{};
		std::optional<FormSummary>  cell;
		std::optional<FormSummary>  location;
		std::optional<FormSummary>  owner;
		bool                        disabled{false};
		bool                        deleted{false};
		std::optional<ActorSummary> actor;
	};

	// Supplies and changes the current selection; calls may complete asynchronously.
	class SelectionSource
	{
	public:
		virtual ~SelectionSource() = default;

		// Any thread; coalesces with an in-flight refresh.
		virtual void Refresh() = 0;

		// Any thread; selects the next reference under the mouse cursor and refreshes.
		virtual void SelectAtCursor() = 0;

		// Null when nothing is selected or before the first refresh.
		[[nodiscard]] virtual std::shared_ptr<const Selection> Current() const = 0;
	};
}
