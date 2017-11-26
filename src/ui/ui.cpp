#include "CreaturePanel.hpp"

#include "Label.hpp"
#include "Meter.hpp"
#include "../app/Assets.hpp"
#include "../creature/Creature.hpp"
#include "../creature/Need.hpp"
#include "../graphics/Viewport.hpp"

#include <iomanip>
#include <sstream>
#include <glm/gtx/transform.hpp>


namespace blobs {
namespace ui {

CreaturePanel::CreaturePanel(const app::Assets &assets)
: assets(assets)
, c(nullptr)
, name(new Label(assets.fonts.large))
, age(new Label(assets.fonts.medium))
, mass(new Label(assets.fonts.medium))
, goal(new Label(assets.fonts.medium))
, needs(new Panel)
, panel()
, health_meter(new Meter)
, need_meters() {
	Label *health_label = new Label(assets.fonts.medium);
	health_label->Text("Health");
	health_meter
		->Size(glm::vec2(100.0f, assets.fonts.medium.Height() + assets.fonts.medium.Descent()))
		->Padding(glm::vec2(1.0f))
		->Border(1.0f)
		->FillColor(glm::vec4(0.9f, 0.0f, 0.0f, 1.0f))
		->BorderColor(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
	Panel *health_panel = new Panel;
	health_panel
		->Add(health_label)
		->Add(health_meter)
		->Spacing(10.0f)
		->Direction(Panel::HORIZONTAL);

	Label *age_label = new Label(assets.fonts.medium);
	age_label->Text("Age");
	Panel *age_panel = new Panel;
	age_panel
		->Add(age_label)
		->Add(age)
		->Spacing(10.0f)
		->Direction(Panel::HORIZONTAL);

	Label *mass_label = new Label(assets.fonts.medium);
	mass_label->Text("Mass");
	Panel *mass_panel = new Panel;
	mass_panel
		->Add(mass_label)
		->Add(mass)
		->Spacing(10.0f)
		->Direction(Panel::HORIZONTAL);

	Label *goal_label = new Label(assets.fonts.medium);
	goal_label->Text("Goal");
	Panel *goal_panel = new Panel;
	goal_panel
		->Add(goal_label)
		->Add(goal)
		->Spacing(10.0f)
		->Direction(Panel::HORIZONTAL);

	panel
		.Add(name)
		->Add(age_panel)
		->Add(mass_panel)
		->Add(goal_panel)
		->Add(health_panel)
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
	need_meters.clear();
	need_meters.reserve(c->Needs().size());
	for (auto &need : c->Needs()) {
		Label *label = new Label(assets.fonts.medium);
		label->Text(need->name);
		Meter *meter = new Meter;
		meter
			->Value(1.0f - need->value)
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
		need_meters.push_back(meter);
	}
	panel.Relayout();
}

void CreaturePanel::Hide() noexcept {
	c = nullptr;
}

void CreaturePanel::Draw(app::Assets &assets, graphics::Viewport &viewport) noexcept {
	if (!c) return;

	age->Text(std::to_string(int(c->Age())) + "s");
	{
		std::stringstream ss;
		ss << std::fixed << std::setprecision(3) << c->Mass() << "kg";
		mass->Text(ss.str());
	}
	if (c->Goals().empty()) {
		goal->Text("none");
	} else {
		goal->Text(c->Goals()[0]->Describe());
	}
	health_meter->Value(c->Health());

	if (need_meters.size() != c->Needs().size()) {
		CreateNeeds();
	} else {
		auto need = c->Needs().begin();
		auto need_end = c->Needs().end();
		auto meter = need_meters.begin();
		for (; need != need_end; ++need, ++meter) {
			(*meter)->Value(1.0f - (*need)->value);
			if ((*need)->IsSatisfied()) {
				(*meter)->FillColor(glm::vec4(0.0f, 0.7f, 0.0f, 1.0f));
			} else if ((*need)->IsInconvenient()) {
				(*meter)->FillColor(glm::vec4(0.7f, 0.5f, 0.0f, 1.0f));
			} else {
				(*meter)->FillColor(glm::vec4(0.9f, 0.0f, 0.0f, 1.0f));
			}
		}
	}

	const glm::vec2 margin(20.0f);

	panel.Position(glm::vec2(viewport.Width() - margin.x - panel.Size().x, margin.y));
	panel.Draw(assets, viewport);
}

}
}
