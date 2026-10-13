#include "Game/PickedReference.h"

#include "Game/CursorPick.h"

namespace Game
{
	namespace
	{
		[[nodiscard]] std::string FileName(const RE::TESFile* a_file)
		{
			return a_file ? std::string{a_file->GetFilename()} : std::string{};
		}

		[[nodiscard]] Console::FormSummary Summarize(const RE::TESForm& a_form)
		{
			const auto* editorID = a_form.GetFormEditorID();
			return {
				.formID = a_form.GetFormID(),
				.type = a_form.GetFormTypeString(),
				.name = std::string{RE::TESFullName::GetFullName(a_form)},
				.editorID = editorID ? editorID : "",
				.definedIn = FileName(a_form.GetFile(0)),
				.lastChangedBy = FileName(a_form.GetFile())};
		}

		[[nodiscard]] std::optional<Console::FormSummary> SummarizeIf(const RE::TESForm* a_form)
		{
			return a_form ? std::optional{Summarize(*a_form)} : std::nullopt;
		}

		[[nodiscard]] std::optional<Console::ActorSummary> SummarizeActor(RE::TESObjectREFR& a_reference)
		{
			auto* actor = a_reference.As<RE::Actor>();
			if (!actor) {
				return std::nullopt;
			}

			Console::ActorSummary summary{
				.level = actor->GetLevel(),
				.race = actor->race ? std::string{RE::TESFullName::GetFullName(*actor->race)} : std::string{}};
			if (const auto* values = RE::ActorValue::GetSingleton(); values && values->health) {
				summary.health = actor->GetActorValue(*values->health);
				summary.healthMax = actor->GetPermanentActorValue(*values->health);
			}
			return summary;
		}

		[[nodiscard]] std::shared_ptr<const Console::Selection> ReadPick()
		{
			const auto reference = RE::Console::GetPickRef().get();
			if (!reference) {
				return nullptr;
			}

			auto  selection = std::make_shared<Console::Selection>();
			auto& refr = *reference;
			selection->reference = Summarize(refr);
			// References have no full name of their own; this resolves renames and the base name.
			if (const auto* name = refr.GetDisplayFullName()) {
				selection->reference.name = name;
			}
			if (const auto* base = refr.GetObjectReference()) {
				selection->base = Summarize(*base);
			}
			const auto& location = refr.data.location;
			selection->position = {location.x, location.y, location.z};
			selection->cell = SummarizeIf(refr.GetParentCell());
			selection->location = SummarizeIf(refr.GetCurrentLocation());
			selection->owner = SummarizeIf(refr.GetOwner());
			selection->disabled = refr.IsDisabled();
			selection->deleted = refr.IsDeleted();
			selection->actor = SummarizeActor(refr);
			return selection;
		}
	}

	void PickedReference::Refresh()
	{
		const auto tasks = F4SE::GetTaskInterface();
		if (!tasks || pending_.exchange(true)) {
			return;
		}

		// Forms are only safe to read on the game thread.
		tasks->AddTask([this] {
			current_.store(ReadPick());
			pending_ = false;
		});
	}

	void PickedReference::SelectAtCursor()
	{
		const auto tasks = F4SE::GetTaskInterface();
		if (!tasks) {
			return;
		}

		tasks->AddTask([this] {
			CursorPick::PickNextAtCursor();
			current_.store(ReadPick());
		});
	}

	std::shared_ptr<const Console::Selection> PickedReference::Current() const
	{
		return current_.load();
	}
}
