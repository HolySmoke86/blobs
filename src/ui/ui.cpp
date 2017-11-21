#include "CreaturePanel.hpp"

#include "Label.hpp"
#include "Meter.hpp"
#include "../app/Assets.hpp"
#include "../creature/Creature.hpp"
#include "../graphics/Viewport.hpp"

#include <glm/gtx/transform.hpp>


namespace blobs {
namespace ui {

CreaturePanel::CreaturePanel(const app::Assets &assets)
: assets(assets)
, c(nullptr)
, name(new Label(assets.fonts.large))
, needs(new Panel)
, panel() {
	panel
		.Add(name)
		->Add(needs)
		->Padding(glm::vec2(10.0f))
		->Spacing(10.0f)
		->Direction(Panel::VERTICAL)
		->Background(glm::vec4(0.7f, 0.7f, 0.7f, 1.0f));
}

CreaturePanel::~CreaturePanel() {
}


void CreaturePanel::Show(creature::Creature &cr) {
	c = &cr;
	name->Text(c->Name());
	CreateNeeds();
}

void CreaturePanel::CreateNeeds() {
	needs->Clear()->Reserve(c->Needs().size());
	meters.clear();
	meters.reserve(c->Needs().size());
	for (auto &need : c->Needs()) {
		Label *label = new Label(assets.fonts.medium);
		label
			->Text(assets.data.resources[need.resource].label);
		Meter *meter = new Meter;
		meter
			->Value(1.0f - need.value)
			->Size(glm::vec2(100.0f, assets.fonts.medium.Height() + assets.fonts.medium.Descent()))
			->Padding(glm::vec2(1.0f))
			->Border(1.0f)
			->FillColor(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f))
			->BorderColor(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
		Panel *need_panel = new Panel;
		need_panel
			->Direction(Panel::HORIZONTAL)
			->Spacing(10.0f)
			->Add(label)
			->Add(meter);
		needs->Add(need_panel);
		meters.push_back(meter);
	}
	panel.Relayout();
}

void CreaturePanel::Hide() noexcept {
	c = nullptr;
}

void CreaturePanel::Draw(app::Assets &assets, graphics::Viewport &viewport) noexcept {
	if (!c) return;

	if (meters.size() != c->Needs().size()) {
		CreateNeeds();
	} else {
		auto need = c->Needs().begin();
		auto need_end = c->Needs().end();
		auto meter = meters.begin();
		for (; need != need_end; ++need, ++meter) {
			(*meter)->Value(1.0f - need->value)->FillColor(need->IsSatisfied()
				? glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)
				: glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
		}
	}

	const glm::vec2 margin(20.0f);

	panel.Position(glm::vec3(viewport.Width() - margin.x - panel.Size().x, margin.y, 0.0f));
	panel.Draw(assets, viewport);
}

}
}
