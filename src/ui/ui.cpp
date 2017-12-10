#include "BodyPanel.hpp"
#include "CreaturePanel.hpp"
#include "RecordsPanel.hpp"
#include "string.hpp"
#include "TimePanel.hpp"

#include "Label.hpp"
#include "Meter.hpp"
#include "../app/Assets.hpp"
#include "../creature/Creature.hpp"
#include "../graphics/Viewport.hpp"
#include "../math/const.hpp"
#include "../world/Body.hpp"
#include "../world/Simulation.hpp"

#include <iomanip>
#include <iostream>
#include <sstream>
#include <glm/gtx/io.hpp>
#include <glm/gtx/transform.hpp>


namespace blobs {
namespace ui {

CreaturePanel::CreaturePanel(app::Assets &assets)
: assets(assets)
, c(nullptr)
, name(new Label(assets.fonts.large))
, parents(new Label(assets.fonts.medium))
, born(new Label(assets.fonts.medium))
, age(new Label(assets.fonts.medium))
, mass(new Label(assets.fonts.medium))
, size(new Label(assets.fonts.medium))
, goal(new Label(assets.fonts.medium))
, pos(new Label(assets.fonts.medium))
, tile(new Label(assets.fonts.medium))
, head(new Label(assets.fonts.medium))
, composition(new Panel)
, stats{nullptr}
, props{nullptr}
, panel() {
	Label *parents_label = new Label(assets.fonts.medium);
	parents_label->Text("Parents");
	Label *born_label = new Label(assets.fonts.medium);
	born_label->Text("Born");
	Label *age_label = new Label(assets.fonts.medium);
	age_label->Text("Age");
	Label *mass_label = new Label(assets.fonts.medium);
	mass_label->Text("Mass");
	Label *size_label = new Label(assets.fonts.medium);
	size_label->Text("Size");
	Label *goal_label = new Label(assets.fonts.medium);
	goal_label->Text("Goal");

	Panel *info_label_panel = new Panel;
	info_label_panel
		->Direction(Panel::VERTICAL)
		->Add(parents_label)
		->Add(born_label)
		->Add(age_label)
		->Add(mass_label)
		->Add(size_label)
		->Add(goal_label);
	Panel *info_value_panel = new Panel;
	info_value_panel
		->Direction(Panel::VERTICAL)
		->Add(parents)
		->Add(born)
		->Add(age)
		->Add(mass)
		->Add(size)
		->Add(goal);
	Panel *info_panel = new Panel;
	info_panel
		->Direction(Panel::HORIZONTAL)
		->Spacing(10.0f)
		->Add(info_label_panel)
		->Add(info_value_panel);

	Label *pos_label = new Label(assets.fonts.medium);
	pos_label->Text("Pos");
	Label *tile_label = new Label(assets.fonts.medium);
	tile_label->Text("Tile");
	Label *head_label = new Label(assets.fonts.medium);
	head_label->Text("Heading");

	Panel *loc_label_panel = new Panel;
	loc_label_panel
		->Direction(Panel::VERTICAL)
		->Add(pos_label)
		->Add(tile_label)
		->Add(head_label);
	Panel *loc_value_panel = new Panel;
	loc_value_panel
		->Direction(Panel::VERTICAL)
		->Add(pos)
		->Add(tile)
		->Add(head);
	Panel *loc_panel = new Panel;
	loc_panel
		->Direction(Panel::HORIZONTAL)
		->Spacing(10.0f)
		->Add(loc_label_panel)
		->Add(loc_value_panel);

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
	stat_label[0]->Layout();
	stats[0]->Layout();
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

	Label *prop_label[9];
	for (int i = 0; i < 9; ++i) {
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
	prop_label[7]->Text("Adaptability");
	prop_label[8]->Text("Offspring mass");

	Panel *prop_label_panel = new Panel;
	prop_label_panel
		->Spacing(2)
		->Direction(Panel::VERTICAL);
	Panel *prop_meter_panel = new Panel;
	prop_meter_panel
		->Spacing(2)
		->Direction(Panel::VERTICAL);
	for (int i = 0; i < 9; ++i) {
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
		->Add(info_panel)
		->Add(loc_panel)
		->Add(composition)
		->Add(stat_panel)
		->Add(prop_panel)
		->Padding(glm::vec2(10.0f))
		->Spacing(10.0f)
		->Direction(Panel::VERTICAL)
		->Background(glm::vec4(1.0f, 1.0f, 1.0f, 0.7f));
}

CreaturePanel::~CreaturePanel() {
}


void CreaturePanel::Show(creature::Creature &cr) {
	c = &cr;
	born->Text(TimeString(c->Born()));

	if (c->Parents().empty()) {
		parents->Text("none");
	} else {
		std::string parent_string;
		bool first = true;
		for (auto p : c->Parents()) {
			if (first) {
				first = false;
			} else {
				parent_string += " and ";
			}
			parent_string += p->Name();
		}
		parents->Text(parent_string);
	}
}

void CreaturePanel::Hide() noexcept {
	c = nullptr;
}

void CreaturePanel::Draw(graphics::Viewport &viewport) noexcept {
	if (!c) return;

	name->Text(c->Name());
	age->Text(TimeString(c->Age()));
	mass->Text(MassString(c->Mass()));
	size->Text(LengthString(c->Size()));
	if (c->Goals().empty()) {
		goal->Text("none");
	} else {
		goal->Text(c->Goals()[0]->Describe());
	}

	pos->Text(VectorString(c->GetSituation().Position(), 2));
	tile->Text(c->GetSituation().GetTileType().label + (
		c->GetSituation().OnGround()
			? (c->GetSituation().Moving() ? " (moving)" : " (standing)")
			: (c->GetSituation().Moving() ? " (flying)" : " (hovering)")
	));
	head->Text(VectorString(c->GetSituation().Heading(), 2));

	const creature::Composition &comp = c->GetComposition();
	if (comp.size() < components.size()) {
		components.clear();
		composition->Clear();
	}
	while (comp.size() > components.size()) {
		components.emplace_back(new Label(assets.fonts.medium));
		composition->Add(components.back());
	}
	{
		int i = 0;
		for (auto &cmp : comp) {
			components[i]->Text(
				PercentageString(cmp.value / comp.TotalMass())
				+ " " + assets.data.resources[cmp.resource].label);
			++i;
		}
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

	props[0]->Text(DecimalString(c->Strength(), 2) + " / " + DecimalString(c->GetProperties().Strength(), 2));
	props[1]->Text(DecimalString(c->Stamina(), 2) + " / " + DecimalString(c->GetProperties().Stamina(), 2));
	props[2]->Text(DecimalString(c->Dexerty(), 2) + " / " + DecimalString(c->GetProperties().Dexerty(), 2));
	props[3]->Text(DecimalString(c->Intelligence(), 2) + " / " + DecimalString(c->GetProperties().Intelligence(), 2));
	props[4]->Text(TimeString(c->Lifetime()));
	props[5]->Text(PercentageString(c->Fertility()) + " / " + PercentageString(c->GetProperties().Fertility()));
	props[6]->Text(PercentageString(c->Mutability()));
	props[7]->Text(PercentageString(c->Adaptability()));
	props[8]->Text(MassString(c->OffspringMass()));

	const glm::vec2 margin(20.0f);
	panel.Position(glm::vec2(viewport.Width() - margin.x - panel.Size().x, margin.y));
	panel.Layout();
	panel.Draw(assets, viewport);
}


BodyPanel::BodyPanel(app::Assets &assets)
: assets(assets)
, body(nullptr)
, name(new Label(assets.fonts.large))
, mass(new Label(assets.fonts.medium))
, radius(new Label(assets.fonts.medium))
, soi(new Label(assets.fonts.medium))
, operiod(new Label(assets.fonts.medium))
, rperiod(new Label(assets.fonts.medium))
, atm(new Label(assets.fonts.medium))
, sma(new Label(assets.fonts.medium))
, ecc(new Label(assets.fonts.medium))
, inc(new Label(assets.fonts.medium))
, asc(new Label(assets.fonts.medium))
, arg(new Label(assets.fonts.medium))
, mna(new Label(assets.fonts.medium))
, panel() {
	Label *mass_label = new Label(assets.fonts.medium);
	mass_label->Text("Mass");
	Label *radius_label = new Label(assets.fonts.medium);
	radius_label->Text("Radius");
	Label *soi_label = new Label(assets.fonts.medium);
	soi_label->Text("SOI");
	Label *operiod_label = new Label(assets.fonts.medium);
	operiod_label->Text("Orb. period");
	Label *rperiod_label = new Label(assets.fonts.medium);
	rperiod_label->Text("Rot. period");
	Label *atm_label = new Label(assets.fonts.medium);
	atm_label->Text("Atmosphere");
	Label *sma_label = new Label(assets.fonts.medium);
	sma_label->Text("SMA");
	Label *ecc_label = new Label(assets.fonts.medium);
	ecc_label->Text("Eccentricity");
	Label *inc_label = new Label(assets.fonts.medium);
	inc_label->Text("Inclination");
	Label *asc_label = new Label(assets.fonts.medium);
	asc_label->Text("Asc. node");
	Label *arg_label = new Label(assets.fonts.medium);
	arg_label->Text("Arg. Periapsis");
	Label *mna_label = new Label(assets.fonts.medium);
	mna_label->Text("Mean anomaly");
	Panel *label_panel = new Panel;
	label_panel
		->Direction(Panel::VERTICAL)
		->Add(mass_label)
		->Add(radius_label)
		->Add(soi_label)
		->Add(operiod_label)
		->Add(rperiod_label)
		->Add(atm_label)
		->Add(sma_label)
		->Add(ecc_label)
		->Add(inc_label)
		->Add(asc_label)
		->Add(arg_label)
		->Add(mna_label);
	Panel *value_panel = new Panel;
	value_panel
		->Direction(Panel::VERTICAL)
		->Add(mass)
		->Add(radius)
		->Add(soi)
		->Add(operiod)
		->Add(rperiod)
		->Add(atm)
		->Add(sma)
		->Add(ecc)
		->Add(inc)
		->Add(asc)
		->Add(arg)
		->Add(mna);
	Panel *info_panel = new Panel;
	info_panel
		->Direction(Panel::HORIZONTAL)
		->Spacing(10.0f)
		->Add(label_panel)
		->Add(value_panel);

	panel
		.Add(name)
		->Add(info_panel)
		->Padding(glm::vec2(10.0f))
		->Spacing(10.0f)
		->Direction(Panel::VERTICAL)
		->Background(glm::vec4(1.0f, 1.0f, 1.0f, 0.7f));
}

BodyPanel::~BodyPanel() {
}

void BodyPanel::Show(world::Body &b) {
	body = &b;
	name->Text(b.Name());
	mass->Text(MassString(b.Mass()));
	radius->Text(LengthString(b.Radius()));
	soi->Text(LengthString(b.SphereOfInfluence()));
	operiod->Text(TimeString(b.OrbitalPeriod()));
	rperiod->Text(TimeString(b.RotationalPeriod()));
	atm->Text(b.HasAtmosphere() ? assets.data.resources[b.Atmosphere()].label : std::string("none"));
	sma->Text(LengthString(b.GetOrbit().SemiMajorAxis()));
	ecc->Text(DecimalString(b.GetOrbit().Eccentricity(), 3));
	inc->Text(AngleString(b.GetOrbit().Inclination()));
	asc->Text(AngleString(b.GetOrbit().LongitudeAscending()));
	arg->Text(AngleString(b.GetOrbit().ArgumentPeriapsis()));
	mna->Text(AngleString(b.GetOrbit().MeanAnomaly()));
}

void BodyPanel::Hide() noexcept {
	body = nullptr;
}

void BodyPanel::Draw(graphics::Viewport &viewport) noexcept {
	if (!Shown()) return;

	const glm::vec2 margin(20.0f);
	panel.Position(glm::vec2(viewport.Width() - margin.x - panel.Size().x, margin.y));
	panel.Layout();
	panel.Draw(assets, viewport);
}


RecordsPanel::RecordsPanel(world::Simulation &sim)
: sim(sim)
, records()
, holders()
, panel()
, shown(true) {
	Label *rank_label = new Label(sim.Assets().fonts.medium);
	rank_label->Text("Rank");

	Panel *rank_panel = new Panel;
	rank_panel
		->Direction(Panel::VERTICAL)
		->Add(rank_label);

	for (int i = 0; i < world::Record::MAX; ++i) {
		rank_label = new Label(sim.Assets().fonts.medium);
		rank_label->Text(std::to_string(i + 1));
		rank_panel->Add(rank_label);
	}

	panel
		.Direction(Panel::HORIZONTAL)
		->Padding(glm::vec2(10.0f))
		->Spacing(45.0f)
		->Background(glm::vec4(1.0f, 1.0f, 1.0f, 0.7f))
		->Add(rank_panel);

	records.reserve(sim.Records().size() * (world::Record::MAX + 1));
	holders.reserve(sim.Records().size() * (world::Record::MAX + 1));
	int ri = 0;
	for (const auto &r : sim.Records()) {
		Panel *by_panel = new Panel;
		by_panel
			->Direction(Panel::VERTICAL);
		Panel *val_panel = new Panel;
		val_panel
			->Direction(Panel::VERTICAL);
		Panel *tab_panel = new Panel;
		tab_panel
			->Direction(Panel::HORIZONTAL)
			->Spacing(10.0f)
			->Add(by_panel)
			->Add(val_panel);
		Label *rec_label = new Label(sim.Assets().fonts.medium);
		rec_label->Text(r.name);
		Panel *rec_panel = new Panel;
		rec_panel
			->Direction(Panel::VERTICAL)
			->Add(rec_label)
			->Add(tab_panel);
		for (int i = 0; i < world::Record::MAX; ++i) {
			Label *val_label = new Label(sim.Assets().fonts.medium);
			val_panel->Add(val_label);
			records.push_back(val_label);
			Label *holder_label = new Label(sim.Assets().fonts.medium);
			by_panel->Add(holder_label);
			holders.push_back(holder_label);
		}
		Panel *group_panel = new Panel;
		group_panel
			->Direction(Panel::HORIZONTAL)
			->Spacing(10.0f)
			->Add(rec_panel);
		panel.Add(group_panel);
		++ri;
	}
}

RecordsPanel::~RecordsPanel() {
}

void RecordsPanel::Draw(graphics::Viewport &viewport) noexcept {
	if (!shown) return;

	int ri = 0;
	for (const auto &r : sim.Records()) {
		for (int i = 0; i < world::Record::MAX; ++i) {
			if (!r.rank[i]) {
				break;
			}
			records[ri * world::Record::MAX + i]->Text(r.ValueString(i));
			holders[ri * world::Record::MAX + i]->Text(r.rank[i].holder->Name());
		}
		++ri;
	}

	const glm::vec2 margin(20.0f);
	panel.Position(glm::vec2(margin.x, margin.y));
	panel.Layout();
	panel.Draw(sim.Assets(), viewport);
}


TimePanel::TimePanel(world::Simulation &sim)
: sim(sim)
, body(nullptr)
, live(new Label(sim.Assets().fonts.medium))
, time(new Label(sim.Assets().fonts.medium))
, clock(new Label(sim.Assets().fonts.medium))
, panel() {
	Label *live_label = new Label(sim.Assets().fonts.medium);
	live_label->Text("Alive");
	Label *time_label = new Label(sim.Assets().fonts.medium);
	time_label->Text("Time");
	Label *clock_label = new Label(sim.Assets().fonts.medium);
	clock_label->Text("Clock");

	Panel *label_panel = new Panel;
	label_panel
		->Direction(Panel::VERTICAL)
		->Add(live_label)
		->Add(time_label)
		->Add(clock_label);

	Panel *value_panel = new Panel;
	value_panel
		->Direction(Panel::VERTICAL)
		->Add(live)
		->Add(time)
		->Add(clock);

	panel
		.Direction(Panel::HORIZONTAL)
		->Padding(glm::vec2(10.0f))
		->Spacing(10.0f)
		->Background(glm::vec4(1.0f, 1.0f, 1.0f, 0.7f))
		->Add(label_panel)
		->Add(value_panel);
}

TimePanel::~TimePanel() {
}

void TimePanel::Draw(graphics::Viewport &viewport) noexcept {
	live->Text(NumberString(sim.LiveCreatures().size()));
	time->Text(TimeString(sim.Time()));
	if (body) {
		clock->Text(TimeString(std::fmod(sim.Time(), body->RotationalPeriod())));
	} else {
		clock->Text("no reference");
	}

	const glm::vec2 margin(20.0f);
	panel.Position(glm::vec2(margin.x, viewport.Height() - margin.y - panel.Size().y));
	panel.Layout();
	panel.Draw(sim.Assets(), viewport);
}


namespace {
std::ostream &custom_fixed(std::ostream &out, double n, int d) {
	double f = n;
	int p = d;
	while (f > 1.0 && p > 0) {
		--p;
		f *= 0.1;
	}
	if (p > 0) {
		out << std::fixed << std::setprecision(p) << n;
	} else {
		out << std::defaultfloat << std::setprecision(d) << n;
	}
	return out;
}
}

std::string AngleString(double a) {
	std::stringstream s;
	s << std::fixed << std::setprecision(2) << std::fmod(a * 180.0 * PI_inv, 360.0) << "°";
	return s.str();
}

std::string DecimalString(double n, int p) {
	std::stringstream s;
	s << std::fixed << std::setprecision(p) << n;
	return s.str();
}

std::string LengthString(double m) {
	std::stringstream s;
	if (m > 1.5e9) {
		custom_fixed(s, m * 1.0e-9, 4) << "Gm";
	} else if (m > 1.5e6) {
		custom_fixed(s, m * 1.0e-6, 4) << "Mm";
	} else if (m > 1.5e3) {
		custom_fixed(s, m * 1.0e-3, 4) << "km";
	} else if (m < 1.5e-3) {
		custom_fixed(s, m * 1.0e3, 4) << "mm";
	} else if (m < 1.5) {
		custom_fixed(s, m * 1.0e2, 4) << "cm";
	} else {
		custom_fixed(s, m, 4) << "m";
	}
	return s.str();
}

std::string MassString(double kg) {
	std::stringstream s;
	if (kg > 1.5e12) {
		custom_fixed(s, kg * 1.0e-12, 4) << "Gt";
	} else if (kg > 1.5e9) {
		custom_fixed(s, kg * 1.0e-9, 4) << "Mt";
	} else if (kg > 1.5e6) {
		custom_fixed(s, kg * 1.0e-6, 4) << "kt";
	} else if (kg > 1.5e3) {
		custom_fixed(s, kg * 1.0e-3, 4) << "t";
	} else if (kg < 1.0e-3) {
		custom_fixed(s, kg * 1.0e6, 4) << "mg";
	} else if (kg < 1.0) {
		custom_fixed(s, kg * 1.0e3, 4) << "g";
	} else {
		custom_fixed(s, kg, 4) << "kg";
	}
	return s.str();
}

std::string NumberString(int n) {
	return std::to_string(n);
}

std::string PercentageString(double n) {
	std::stringstream s;
	s << std::fixed << std::setprecision(1) << (n * 100.0) << '%';
	return s.str();
}

std::string TimeString(double s) {
	int is = int(s);
	int md = 1;
	int sd = 1;
	std::stringstream ss;
	ss << std::setfill('0');
	if (is >= 3600) {
		ss << (is / 3600) << "h ";
		is %= 3600;
		md = 2;
	}
	if (is >= 60) {
		ss << std::setw(md) << (is / 60) << "m ";
		is %= 60;
		sd = 2;
	}
	ss << std::setw(sd) << is << 's';
	return ss.str();
}

std::string VectorString(const glm::dvec3 &v, int p) {
	std::stringstream ss;
	ss << std::fixed << std::setprecision(p)
		<< "<" << v.x << ", " << v.y << ", " << v.z << ">";
	return ss.str();
}

std::string VectorString(const glm::ivec2 &v) {
	std::stringstream ss;
	ss << "<" << v.x << ", " << v.y << ">";
	return ss.str();
}

}
}
