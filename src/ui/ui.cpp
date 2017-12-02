#include "CreaturePanel.hpp"

#include "Label.hpp"
#include "Meter.hpp"
#include "../app/Assets.hpp"
#include "../creature/Creature.hpp"
#include "../graphics/Viewport.hpp"

#include <iomanip>
#include <iostream>
#include <sstream>
#include <glm/gtx/io.hpp>
#include <glm/gtx/transform.hpp>


namespace blobs {
namespace ui {

CreaturePanel::CreaturePanel(const app::Assets &assets)
: assets(assets)
, c(nullptr)
, name(new Label(assets.fonts.large))
, born(new Label(assets.fonts.medium))
, age(new Label(assets.fonts.medium))
, mass(new Label(assets.fonts.medium))
, pos(new Label(assets.fonts.medium))
, vel(new Label(assets.fonts.medium))
, dir(new Label(assets.fonts.medium))
, tile(new Label(assets.fonts.medium))
, goal(new Label(assets.fonts.medium))
, stats{nullptr}
, props{nullptr}
, panel() {
	born->Text("00h 00m 00s");
	Label *born_label = new Label(assets.fonts.medium);
	born_label->Text("Born");
	Panel *born_panel = new Panel;
	born_panel
		->Add(born_label)
		->Add(born)
		->Spacing(10.0f)
		->Direction(Panel::HORIZONTAL);

	age->Text("00h 00m 00s");
	Label *age_label = new Label(assets.fonts.medium);
	age_label->Text("Age");
	Panel *age_panel = new Panel;
	age_panel
		->Add(age_label)
		->Add(age)
		->Spacing(10.0f)
		->Direction(Panel::HORIZONTAL);

	mass->Text("00.000kg");
	Label *mass_label = new Label(assets.fonts.medium);
	mass_label->Text("Mass");
	Panel *mass_panel = new Panel;
	mass_panel
		->Add(mass_label)
		->Add(mass)
		->Spacing(10.0f)
		->Direction(Panel::HORIZONTAL);

	pos->Text("<00.0, 00.0, 00.0>");
	Label *pos_label = new Label(assets.fonts.medium);
	pos_label->Text("Pos");
	Panel *pos_panel = new Panel;
	pos_panel
		->Add(pos_label)
		->Add(pos)
		->Spacing(10.0f)
		->Direction(Panel::HORIZONTAL);

	vel->Text("<00.0, 00.0, 00.0>");
	Label *vel_label = new Label(assets.fonts.medium);
	vel_label->Text("Vel");
	Panel *vel_panel = new Panel;
	vel_panel
		->Add(vel_label)
		->Add(vel)
		->Spacing(10.0f)
		->Direction(Panel::HORIZONTAL);

	dir->Text("<0.00, 0.00, 0.00>");
	Label *dir_label = new Label(assets.fonts.medium);
	dir_label->Text("Dir");
	Panel *dir_panel = new Panel;
	dir_panel
		->Add(dir_label)
		->Add(dir)
		->Spacing(10.0f)
		->Direction(Panel::HORIZONTAL);

	tile->Text("<00, 00> (mountains)");
	Label *tile_label = new Label(assets.fonts.medium);
	tile_label->Text("Tile");
	Panel *tile_panel = new Panel;
	tile_panel
		->Add(tile_label)
		->Add(tile)
		->Spacing(10.0f)
		->Direction(Panel::HORIZONTAL);

	goal->Text("long goal description");
	Label *goal_label = new Label(assets.fonts.medium);
	goal_label->Text("Goal");
	Panel *goal_panel = new Panel;
	goal_panel
		->Add(goal_label)
		->Add(goal)
		->Spacing(10.0f)
		->Direction(Panel::HORIZONTAL);

	Label *stat_label[7];
	for (int i = 0; i < 7; ++i) {
		stat_label[i] = new Label(assets.fonts.medium);
		stats[i] = new Meter;
		stats[i]
			->Size(glm::vec2(100.0f, assets.fonts.medium.Height() + assets.fonts.medium.Descent()))
			->Padding(glm::vec2(1.0f))
			->Border(1.0f)
			->BorderColor(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
	}
	stat_label[0]->Text("Damage");
	stat_label[1]->Text("Breath");
	stat_label[2]->Text("Thirst");
	stat_label[3]->Text("Hunger");
	stat_label[4]->Text("Exhaustion");
	stat_label[5]->Text("Fatigue");
	stat_label[6]->Text("Boredom");

	Panel *stat_label_panel = new Panel;
	stat_label_panel
		->Spacing(2)
		->Direction(Panel::VERTICAL);
	Panel *stat_meter_panel = new Panel;
	stat_meter_panel
		->Spacing(stat_label[0]->Size().y - stats[0]->Size().y + 2)
		->Direction(Panel::VERTICAL);
	for (int i = 0; i < 7; ++i) {
		stat_label_panel->Add(stat_label[i]);
		stat_meter_panel->Add(stats[i]);
	}
	Panel *stat_panel = new Panel;
	stat_panel
		->Direction(Panel::HORIZONTAL)
		->Spacing(10)
		->Add(stat_label_panel)
		->Add(stat_meter_panel);

	Label *prop_label[8];
	for (int i = 0; i < 8; ++i) {
		prop_label[i] = new Label(assets.fonts.medium);
		props[i] = new Label(assets.fonts.medium);
	}
	prop_label[0]->Text("Strength");
	prop_label[1]->Text("Stamina");
	prop_label[2]->Text("Dexerty");
	prop_label[3]->Text("Intelligence");
	prop_label[4]->Text("Lifetime");
	prop_label[5]->Text("Fertility");
	prop_label[6]->Text("Mutability");
	prop_label[7]->Text("Offspring mass");

	Panel *prop_label_panel = new Panel;
	prop_label_panel
		->Spacing(2)
		->Direction(Panel::VERTICAL);
	Panel *prop_meter_panel = new Panel;
	prop_meter_panel
		->Spacing(2)
		->Direction(Panel::VERTICAL);
	for (int i = 0; i < 8; ++i) {
		prop_label_panel->Add(prop_label[i]);
		prop_meter_panel->Add(props[i]);
	}
	Panel *prop_panel = new Panel;
	prop_panel
		->Direction(Panel::HORIZONTAL)
		->Spacing(10)
		->Add(prop_label_panel)
		->Add(prop_meter_panel);

	panel
		.Add(name)
		->Add(age_panel)
		->Add(born_panel)
		->Add(mass_panel)
		->Add(pos_panel)
		->Add(vel_panel)
		->Add(dir_panel)
		->Add(tile_panel)
		->Add(goal_panel)
		->Add(stat_panel)
		->Add(prop_panel)
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
	born->Time(c->Born());
}

void CreaturePanel::Hide() noexcept {
	c = nullptr;
}

void CreaturePanel::Draw(app::Assets &assets, graphics::Viewport &viewport) noexcept {
	if (!c) return;

	age->Time(c->Age());
	mass->Mass(c->Mass());
	{
		const glm::dvec3 &p = c->GetSituation().Position();
		std::stringstream ss;
		ss << std::fixed << std::setprecision(1)
			<< "<" << p.x << ", " << p.y << ", " << p.z << ">";
		pos->Text(ss.str());
	}
	{
		const glm::dvec3 &v = c->GetSituation().Velocity();
		std::stringstream ss;
		ss << std::fixed << std::setprecision(1)
			<< "<" << v.x << ", " << v.y << ", " << v.z << ">";
		vel->Text(ss.str());
	}
	{
		const glm::dvec3 &d = c->GetSituation().GetState().dir;
		std::stringstream ss;
		ss << std::fixed << std::setprecision(2)
			<< "<" << d.x << ", " << d.y << ", " << d.z << ">";
		dir->Text(ss.str());
	}
	{
		glm::ivec2 t = c->GetSituation().SurfacePosition();
		std::stringstream ss;
		ss << std::fixed << std::setprecision(1)
			<< "<" << t.x << ", " << t.y
			<< "> (" << c->GetSituation().GetTileType().label << ")";
		tile->Text(ss.str());
	}
	if (c->Goals().empty()) {
		goal->Text("none");
	} else {
		goal->Text(c->Goals()[0]->Describe());
	}

	for (int i = 0; i < 7; ++i) {
		stats[i]->Value(c->GetStats().stat[i].value);
		if (c->GetStats().stat[i].Okay()) {
			stats[i]->FillColor(glm::vec4(0.0f, 0.7f, 0.0f, 1.0f));
		} else if (c->GetStats().stat[i].Critical()) {
			stats[i]->FillColor(glm::vec4(0.7f, 0.0f, 0.0f, 1.0f));
		} else {
			stats[i]->FillColor(glm::vec4(0.9f, 0.4f, 0.0f, 1.0f));
		}
	}

	props[0]->Decimal(c->Strength());
	props[1]->Decimal(c->Stamina());
	props[2]->Decimal(c->Dexerty());
	props[3]->Decimal(c->Intelligence());
	props[4]->Time(c->Lifetime());
	props[5]->Percentage(c->Fertility());
	props[6]->Percentage(c->Mutability());
	props[7]->Mass(c->OffspringMass());

	const glm::vec2 margin(20.0f);

	panel.Position(glm::vec2(viewport.Width() - margin.x - panel.Size().x, margin.y));
	panel.Draw(assets, viewport);
}

}
}
