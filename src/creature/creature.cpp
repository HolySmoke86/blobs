#include "Composition.hpp"
#include "Creature.hpp"
#include "Genome.hpp"
#include "Memory.hpp"
#include "NameGenerator.hpp"
#include "Situation.hpp"
#include "Steering.hpp"

#include "BlobBackgroundTask.hpp"
#include "Goal.hpp"
#include "IdleGoal.hpp"
#include "../app/Assets.hpp"
#include "../math/const.hpp"
#include "../world/Body.hpp"
#include "../world/Planet.hpp"
#include "../world/Simulation.hpp"
#include "../world/TileType.hpp"

#include <algorithm>
#include <sstream>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/vector_angle.hpp>

#include <iostream>
#include <glm/gtx/io.hpp>


namespace blobs {
namespace creature {

Composition::Composition()
: components() {
}

Composition::~Composition() {
}

namespace {
bool CompositionCompare(const Composition::Component &a, const Composition::Component &b) {
	return b.value < a.value;
}
}

void Composition::Add(int res, double amount) {
	bool found = false;
	for (auto &c : components) {
		if (c.resource == res) {
			c.value += amount;
			found = true;
			break;
		}
	}
	if (!found) {
		components.emplace_back(res, amount);
	}
	std::sort(components.begin(), components.end(), CompositionCompare);
}

bool Composition::Has(int res) const noexcept {
	for (auto &c : components) {
		if (c.resource == res) {
			return true;
		}
	}
	return false;
}

double Composition::Get(int res) const noexcept {
	for (auto &c : components) {
		if (c.resource == res) {
			return c.value;
		}
	}
	return 0.0;
}


Creature::Creature(world::Simulation &sim)
: sim(sim)
, name()
, genome()
, properties()
, composition()
, base_color(1.0)
, highlight_color(0.0, 0.0, 0.0, 1.0)
, mass(1.0)
, size(1.0)
, birth(sim.Time())
, on_death()
, removable(false)
, stats()
, memory(*this)
, bg_task()
, goals()
, situation()
, steering(*this)
, vao() {
	// all creatures avoid each other for now
	steering.Separate(0.1, 1.5);
}

Creature::~Creature() {
}

void Creature::AddMass(int res, double amount) {
	composition.Add(res, amount);
	double mass = 0.0;
	double nonsolid = 0.0;
	double volume = 0.0;
	for (const auto &c : composition) {
		mass += c.value;
		volume += c.value / sim.Assets().data.resources[c.resource].density;
		if (sim.Assets().data.resources[c.resource].state != world::Resource::SOLID) {
			nonsolid += c.value;
		}
	}
	Mass(mass);
	Size(std::cbrt(volume));
	highlight_color.a = nonsolid / mass;
}

void Creature::HighlightColor(const glm::dvec3 &c) noexcept {
	highlight_color = glm::dvec4(c, highlight_color.a);
}

void Creature::Ingest(int res, double amount) noexcept {
	// TODO: check foreign materials
	// 10% stays in body
	AddMass(res, amount * 0.1);
}

void Creature::Hurt(double amount) noexcept {
	stats.Damage().Add(amount);
	if (stats.Damage().Full()) {
		std::cout << "[" << int(sim.Time()) << "s] " << name << " ";
		if (stats.Exhaustion().Full()) {
			std::cout << "died of exhaustion";
		} else if (stats.Breath().Full()) {
			std::cout << "suffocated";
		} else if (stats.Thirst().Full()) {
			std::cout << "died of thirst";
		} else if (stats.Hunger().Full()) {
			std::cout << "starved to death";
		} else {
			std::cout << "succumed to wounds";
		}
		std::cout << " at an age of ";
		{
			int age = int(Age());
			if (age >= 3600) {
				std::cout << (age / 3600) << "h ";
				age %= 3600;
			}
			if (age >= 60) {
				std::cout << (age / 60) << "m ";
				age %= 60;
			}
			std::cout << age << 's';
		}
		std::cout << " (" << int(Age() / properties.Lifetime() * 100)
			<< "% of life expectancy of ";
		{
			int lt = int(properties.Lifetime());
			if (lt >= 3600) {
				std::cout << (lt / 3600) << "h ";
				lt %= 3600;
			}
			if (lt >= 60) {
				std::cout << (lt / 60) << "m ";
				lt %= 60;
			}
			std::cout << lt << 's';
		}
		std::cout << ")" << std::endl;
		Die();
	}
}

void Creature::Die() noexcept {
	goals.clear();
	steering.Halt();
	if (on_death) {
		on_death(*this);
	}
	Remove();
}

double Creature::Age() const noexcept {
	return sim.Time() - birth;
}

double Creature::AgeFactor(double peak) const noexcept {
	// shifted inverse hermite, y = 1 - (3t² - 2t³) with t = normalized age - peak
	// goes negative below -0.5 and starts to rise again above 1.0
	double t = glm::clamp((Age() / properties.Lifetime()) - peak, -0.5, 1.0);
	return 1.0 - (3.0 * t * t) + (2.0 * t * t * t);
}

double Creature::ExhaustionFactor() const noexcept {
	return 1.0 - (glm::smoothstep(0.5, 1.0, stats.Exhaustion().value) * 0.5);
}

double Creature::FatigueFactor() const noexcept {
	return 1.0 - (glm::smoothstep(0.5, 1.0, stats.Fatigue().value) * 0.5);
}

double Creature::Strength() const noexcept {
	// TODO: replace all age factors with actual growth and decay
	return properties.Strength() * ExhaustionFactor() * AgeFactor(0.25);
}

double Creature::Stamina() const noexcept {
	return properties.Stamina() * ExhaustionFactor() * AgeFactor(0.25);
}

double Creature::Dexerty() const noexcept {
	return properties.Dexerty() * ExhaustionFactor() * AgeFactor(0.25);
}

double Creature::Intelligence() const noexcept {
	return properties.Intelligence() * FatigueFactor() * AgeFactor(0.25);
}

double Creature::Lifetime() const noexcept {
	return properties.Lifetime();
}

double Creature::Fertility() const noexcept {
	return properties.Fertility() * AgeFactor(0.25);
}

double Creature::Mutability() const noexcept {
	return properties.Mutability();
}

double Creature::OffspringMass() const noexcept {
	return properties.OffspringMass();
}

double Creature::OffspringChance() const noexcept {
	return AgeFactor(0.25) * properties.Fertility() * (1.0 / 3600.0);
}

double Creature::MutateChance() const noexcept {
	return GetProperties().Mutability() * (1.0 / 3600.0);
}

void Creature::AddGoal(std::unique_ptr<Goal> &&g) {
	g->Enable();
	goals.emplace_back(std::move(g));
}

namespace {

bool GoalCompare(const std::unique_ptr<Goal> &a, const std::unique_ptr<Goal> &b) {
	return b->Urgency() < a->Urgency();
}

}

void Creature::Tick(double dt) {
	TickState(dt);
	TickStats(dt);
	TickBrain(dt);
}

void Creature::TickState(double dt) {
	steering.MaxSpeed(Dexerty());
	steering.MaxForce(Strength());
	Situation::State state(situation.GetState());
	Situation::Derivative a(Step(Situation::Derivative(), 0.0));
	Situation::Derivative b(Step(a, dt * 0.5));
	Situation::Derivative c(Step(b, dt * 0.5));
	Situation::Derivative d(Step(c, dt));
	Situation::Derivative f(
		(1.0 / 6.0) * (a.vel + 2.0 * (b.vel + c.vel) + d.vel),
		(1.0 / 6.0) * (a.acc + 2.0 * (b.acc + c.acc) + d.acc)
	);
	state.pos += f.vel * dt;
	state.vel += f.acc * dt;
	if (length2(state.vel) > 0.000001) {
		glm::dvec3 nvel(normalize(state.vel));
		double ang = angle(nvel, state.dir);
		double turn_rate = PI * 0.5 * dt;
		if (ang < turn_rate) {
			state.dir = normalize(state.vel);
		} else if (std::abs(ang - PI) < 0.001) {
			state.dir = rotate(state.dir, turn_rate, world::Planet::SurfaceNormal(situation.Surface()));
		} else {
			state.dir = rotate(state.dir, turn_rate, normalize(cross(state.dir, nvel)));
		}
	}
	situation.SetState(state);
	stats.Exhaustion().Add(length(f.acc) * Mass() / Stamina() * dt);
}

Situation::Derivative Creature::Step(const Situation::Derivative &ds, double dt) const noexcept {
	Situation::State s = situation.GetState();
	s.pos += ds.vel * dt;
	s.vel += ds.acc * dt;
	return {
		s.vel,
		steering.Force(s) / Mass()
	};
}

void Creature::TickStats(double dt) {
	for (auto &s : stats.stat) {
		s.Add(s.gain * dt);
	}
	stats.Breath().Add(stats.Breath().gain * stats.Exhaustion().value * dt);
	// TODO: damage values depending on properties
	if (stats.Breath().Full()) {
		constexpr double dps = 1.0 / 4.0;
		Hurt(dps * dt);
	}
	if (stats.Thirst().Full()) {
		constexpr double dps = 1.0 / 32.0;
		Hurt(dps * dt);
	}
	if (stats.Hunger().Full()) {
		constexpr double dps = 1.0 / 128.0;
		Hurt(dps * dt);
	}
}

void Creature::TickBrain(double dt) {
	bg_task->Tick(dt);
	bg_task->Action();
	memory.Tick(dt);
	// do background stuff
	if (goals.empty()) {
		return;
	}
	for (auto &goal : goals) {
		goal->Tick(dt);
	}
	// if active goal can be interrupted, check priorities
	if (goals.size() > 1 && goals[0]->Interruptible()) {
		std::sort(goals.begin(), goals.end(), GoalCompare);
	}
	goals[0]->Action();
	for (auto goal = goals.begin(); goal != goals.end();) {
		if ((*goal)->Complete()) {
			goals.erase(goal);
		} else {
			++goal;
		}
	}
}

glm::dmat4 Creature::LocalTransform() noexcept {
	const double half_size = size * 0.5;
	const glm::dvec3 &pos = situation.Position();
	const glm::dmat3 srf(world::Planet::SurfaceOrientation(situation.Surface()));
	return glm::translate(glm::dvec3(pos.x, pos.y, pos.z + half_size))
		* glm::rotate(glm::orientedAngle(-srf[2], situation.Heading(), srf[1]), srf[1])
		* glm::dmat4(srf)
		* glm::scale(glm::dvec3(half_size, half_size, half_size));
}

void Creature::BuildVAO() {
	vao.Bind();
	vao.BindAttributes();
	vao.EnableAttribute(0);
	vao.EnableAttribute(1);
	vao.EnableAttribute(2);
	vao.AttributePointer<glm::vec3>(0, false, offsetof(Attributes, position));
	vao.AttributePointer<glm::vec3>(1, false, offsetof(Attributes, normal));
	vao.AttributePointer<glm::vec3>(2, false, offsetof(Attributes, texture));
	vao.ReserveAttributes(6 * 4, GL_STATIC_DRAW);
	{
		auto attrib = vao.MapAttributes(GL_WRITE_ONLY);
		const float offset = 1.0f;
		for (int surface = 0; surface < 6; ++surface) {
			const float tex_u_begin = surface < 3 ? 1.0f : 0.0f;
			const float tex_u_end = surface < 3 ? 0.0f : 1.0f;

			attrib[4 * surface + 0].position[(surface + 0) % 3] = -offset;
			attrib[4 * surface + 0].position[(surface + 1) % 3] = -offset;
			attrib[4 * surface + 0].position[(surface + 2) % 3] = surface < 3 ? offset : -offset;
			attrib[4 * surface + 0].normal[(surface + 0) % 3] = 0.0f;
			attrib[4 * surface + 0].normal[(surface + 1) % 3] = 0.0f;
			attrib[4 * surface + 0].normal[(surface + 2) % 3] = surface < 3 ? 1.0f : -1.0f;
			attrib[4 * surface + 0].texture.x = tex_u_begin;
			attrib[4 * surface + 0].texture.y = 1.0f;
			attrib[4 * surface + 0].texture.z = surface;

			attrib[4 * surface + 1].position[(surface + 0) % 3] = -offset;
			attrib[4 * surface + 1].position[(surface + 1) % 3] =  offset;
			attrib[4 * surface + 1].position[(surface + 2) % 3] = surface < 3 ? offset : -offset;
			attrib[4 * surface + 1].normal[(surface + 0) % 3] = 0.0f;
			attrib[4 * surface + 1].normal[(surface + 1) % 3] = 0.0f;
			attrib[4 * surface + 1].normal[(surface + 2) % 3] = surface < 3 ? 1.0f : -1.0f;
			attrib[4 * surface + 1].texture.x = tex_u_end;
			attrib[4 * surface + 1].texture.y = 1.0f;
			attrib[4 * surface + 1].texture.z = surface;

			attrib[4 * surface + 2].position[(surface + 0) % 3] =  offset;
			attrib[4 * surface + 2].position[(surface + 1) % 3] = -offset;
			attrib[4 * surface + 2].position[(surface + 2) % 3] = surface < 3 ? offset : -offset;
			attrib[4 * surface + 2].normal[(surface + 0) % 3] = 0.0f;
			attrib[4 * surface + 2].normal[(surface + 1) % 3] = 0.0f;
			attrib[4 * surface + 2].normal[(surface + 2) % 3] = surface < 3 ? 1.0f : -1.0f;
			attrib[4 * surface + 2].texture.x = tex_u_begin;
			attrib[4 * surface + 2].texture.y = 0.0f;
			attrib[4 * surface + 2].texture.z = surface;

			attrib[4 * surface + 3].position[(surface + 0) % 3] = offset;
			attrib[4 * surface + 3].position[(surface + 1) % 3] = offset;
			attrib[4 * surface + 3].position[(surface + 2) % 3] = surface < 3 ? offset : -offset;
			attrib[4 * surface + 3].normal[(surface + 0) % 3] = 0.0f;
			attrib[4 * surface + 3].normal[(surface + 1) % 3] = 0.0f;
			attrib[4 * surface + 3].normal[(surface + 2) % 3] = surface < 3 ? 1.0f : -1.0f;
			attrib[4 * surface + 3].texture.x = tex_u_end;
			attrib[4 * surface + 3].texture.y = 0.0f;
			attrib[4 * surface + 3].texture.z = surface;
		}
	}
	vao.BindElements();
	vao.ReserveElements(6 * 6, GL_STATIC_DRAW);
	{
		auto element = vao.MapElements(GL_WRITE_ONLY);
		for (int surface = 0; surface < 3; ++surface) {
			element[6 * surface + 0] = 4 * surface + 0;
			element[6 * surface + 1] = 4 * surface + 2;
			element[6 * surface + 2] = 4 * surface + 1;
			element[6 * surface + 3] = 4 * surface + 1;
			element[6 * surface + 4] = 4 * surface + 2;
			element[6 * surface + 5] = 4 * surface + 3;
		}
		for (int surface = 3; surface < 6; ++surface) {
			element[6 * surface + 0] = 4 * surface + 0;
			element[6 * surface + 1] = 4 * surface + 1;
			element[6 * surface + 2] = 4 * surface + 2;
			element[6 * surface + 3] = 4 * surface + 2;
			element[6 * surface + 4] = 4 * surface + 1;
			element[6 * surface + 5] = 4 * surface + 3;
		}
	}
	vao.Unbind();
}

void Creature::Draw(graphics::Viewport &viewport) {
	vao.Bind();
	vao.DrawTriangles(6 * 6);
}


void Spawn(Creature &c, world::Planet &p) {
	p.AddCreature(&c);
	c.GetSituation().SetPlanetSurface(p, 0, p.TileCenter(0, p.SideLength() / 2, p.SideLength() / 2));
	c.GetSituation().Heading(-world::Planet::SurfaceOrientation(0)[2]);

	// probe surrounding area for common resources
	int start = p.SideLength() / 2 - 2;
	int end = start + 5;
	std::map<int, double> yields;
	for (int y = start; y < end; ++y) {
		for (int x = start; x < end; ++x) {
			const world::TileType &t = p.TypeAt(0, x, y);
			for (auto yield : t.resources) {
				yields[yield.resource] += yield.ubiquity;
			}
		}
	}
	int liquid = -1;
	int solid = -1;
	for (auto e : yields) {
		if (c.GetSimulation().Resources()[e.first].state == world::Resource::LIQUID) {
			if (liquid < 0 || e.second > yields[liquid]) {
				liquid = e.first;
			}
		} else if (c.GetSimulation().Resources()[e.first].state == world::Resource::SOLID) {
			if (solid < 0 || e.second > yields[solid]) {
				solid = e.first;
			}
		}
	}

	Genome genome;
	genome.properties.Strength() = { 2.0, 0.1 };
	genome.properties.Stamina() = { 4.0, 0.1 };
	genome.properties.Dexerty() = { 2.0, 0.1 };
	genome.properties.Intelligence() = { 1.0, 0.1 };
	genome.properties.Lifetime() = { 480.0, 60.0 };
	genome.properties.Fertility() = { 0.5, 0.03 };
	genome.properties.Mutability() = { 1.0, 0.1 };
	genome.properties.OffspringMass() = { 0.3, 0.02 };

	glm::dvec3 color_avg(0.0);
	double color_divisor = 0.0;

	if (p.HasAtmosphere()) {
		c.AddMass(p.Atmosphere(), 0.01);
		color_avg += c.GetSimulation().Resources()[p.Atmosphere()].base_color * 0.1;
		color_divisor += 0.1;
	}
	if (liquid > -1) {
		c.AddMass(liquid, 0.3);
		color_avg += c.GetSimulation().Resources()[liquid].base_color * 0.5;
		color_divisor += 0.5;
	}
	if (solid > -1) {
		c.AddMass(solid, 0.1);
		color_avg += c.GetSimulation().Resources()[solid].base_color;
		color_divisor += 1.0;
	}

	if (color_divisor > 0.001) {
		color_avg /= color_divisor;
	}
	glm::dvec3 hsl = rgb2hsl(color_avg);
	genome.base_hue = { hsl.x, 0.01 };
	genome.base_saturation = { hsl.y, 0.01 };
	genome.base_lightness = { hsl.z, 0.01 };

	genome.Configure(c);
}

void Genome::Configure(Creature &c) const {
	c.GetGenome() = *this;

	math::GaloisLFSR &random = c.GetSimulation().Assets().random;

	c.GetProperties() = Instantiate(properties, random);

	// TODO: derive stats from properties
	c.GetStats().Damage().gain = (-1.0 / 100.0);
	c.GetStats().Breath().gain = (1.0 / 5.0);
	c.GetStats().Thirst().gain = (1.0 / 60.0);
	c.GetStats().Hunger().gain = (1.0 / 200.0);
	c.GetStats().Exhaustion().gain = (-1.0 / 100.0);
	c.GetStats().Fatigue().gain = (-1.0 / 100.0);
	c.GetStats().Boredom().gain = (1.0 / 300.0);

	glm::dvec3 base_color(
		std::fmod(base_hue.FakeNormal(random.SNorm()) + 1.0, 1.0),
		glm::clamp(base_saturation.FakeNormal(random.SNorm()), 0.0, 1.0),
		glm::clamp(base_lightness.FakeNormal(random.SNorm()), 0.0, 1.0)
	);
	glm::dvec3 highlight_color(
		std::fmod(base_color.x + 0.5, 1.0),
		1.0 - base_color.y,
		1.0 - base_color.z
	);
	c.BaseColor(hsl2rgb(base_color));
	c.HighlightColor(hsl2rgb(highlight_color));
	c.SetBackgroundTask(std::unique_ptr<Goal>(new BlobBackgroundTask(c)));
	c.AddGoal(std::unique_ptr<Goal>(new IdleGoal(c)));
}


void Split(Creature &c) {
	Creature *a = new Creature(c.GetSimulation());
	const Situation &s = c.GetSituation();
	a->Name(c.GetSimulation().Assets().name.Sequential());
	c.GetGenome().Configure(*a);
	for (const auto &cmp : c.GetComposition()) {
		a->AddMass(cmp.resource, cmp.value * 0.5);
	}
	s.GetPlanet().AddCreature(a);
	// TODO: duplicate situation somehow
	a->GetSituation().SetPlanetSurface(
		s.GetPlanet(), s.Surface(),
		s.Position() + glm::dvec3(0.0, a->Size() + 0.1, 0.0));
	a->BuildVAO();
	std::cout << "[" << int(c.GetSimulation().Time()) << "s] "
		<< a->Name() << " was born" << std::endl;

	Creature *b = new Creature(c.GetSimulation());
	b->Name(c.GetSimulation().Assets().name.Sequential());
	c.GetGenome().Configure(*b);
	for (const auto &cmp : c.GetComposition()) {
		b->AddMass(cmp.resource, cmp.value * 0.5);
	}
	s.GetPlanet().AddCreature(b);
	b->GetSituation().SetPlanetSurface(
		s.GetPlanet(), s.Surface(),
		s.Position() + glm::dvec3(0.0, b->Size() - 0.1, 0.0));
	b->BuildVAO();
	std::cout << "[" << int(c.GetSimulation().Time()) << "s] "
		<< b->Name() << " was born" << std::endl;

	c.Die();
}


Memory::Memory(Creature &c)
: c(c) {
}

Memory::~Memory() {
}

void Memory::Tick(double dt) {
	Situation &s = c.GetSituation();
	if (s.OnTile()) {
		TrackStay({ &s.GetPlanet(), s.Surface(), s.SurfacePosition() }, dt);
	}
}

void Memory::TrackStay(const Location &l, double t) {
	const world::TileType &type = l.planet->TypeAt(l.surface, l.coords.x, l.coords.y);
	auto entry = known_types.find(type.id);
	if (entry != known_types.end()) {
		if (c.GetSimulation().Time() - entry->second.last_been > c.GetProperties().Lifetime() * 0.1) {
			// "it's been ages"
			if (entry->second.time_spent > c.Age() * 0.25) {
				// the place is very familiar
				c.GetStats().Boredom().Add(-0.2);
			} else {
				// infrequent stays
				c.GetStats().Boredom().Add(-0.1);
			}
		}
		entry->second.last_been = c.GetSimulation().Time();
		entry->second.last_loc = l;
		entry->second.time_spent += t;
	} else {
		known_types.emplace(type.id, Stay{
			c.GetSimulation().Time(),
			l,
			c.GetSimulation().Time(),
			l,
			t
		});
		// completely new place, interesting
		// TODO: scale by personality trait
		c.GetStats().Boredom().Add(-0.25);
	}
}


NameGenerator::NameGenerator()
: counter(0) {
}

NameGenerator::~NameGenerator() {
}

std::string NameGenerator::Sequential() {
	std::stringstream ss;
	ss << "Blob " << ++counter;
	return ss.str();
}


Situation::Situation()
: planet(nullptr)
, state(glm::dvec3(0.0), glm::dvec3(0.0))
, surface(0)
, type(LOST) {
}

Situation::~Situation() {
}

bool Situation::OnPlanet() const noexcept {
	return type == PLANET_SURFACE;
}

bool Situation::OnSurface() const noexcept {
	return type == PLANET_SURFACE;
}

bool Situation::OnTile() const noexcept {
	glm::ivec2 t(planet->SurfacePosition(surface, state.pos));
	return type == PLANET_SURFACE
		&& t.x >= 0 && t.x < planet->SideLength()
		&& t.y >= 0 && t.y < planet->SideLength();
}

glm::ivec2 Situation::SurfacePosition() const noexcept {
	return planet->SurfacePosition(surface, state.pos);
}

world::Tile &Situation::GetTile() const noexcept {
	glm::ivec2 t(planet->SurfacePosition(surface, state.pos));
	return planet->TileAt(surface, t.x, t.y);
}

const world::TileType &Situation::GetTileType() const noexcept {
	glm::ivec2 t(planet->SurfacePosition(surface, state.pos));
	return planet->TypeAt(surface, t.x, t.y);
}

void Situation::Move(const glm::dvec3 &dp) noexcept {
	state.pos += dp;
	if (OnSurface()) {
		// enforce ground constraint
		if (Surface() < 3) {
			state.pos[(Surface() + 2) % 3] = std::max(0.0, state.pos[(Surface() + 2) % 3]);
		} else {
			state.pos[(Surface() + 2) % 3] = std::min(0.0, state.pos[(Surface() + 2) % 3]);
		}
	}
}

void Situation::SetPlanetSurface(world::Planet &p, int srf, const glm::dvec3 &pos) noexcept {
	type = PLANET_SURFACE;
	planet = &p;
	surface = srf;
	state.pos = pos;
}


Steering::Steering(const Creature &c)
: c(c)
, target(0.0)
, haste(0.0)
, max_force(1.0)
, max_speed(1.0)
, min_dist(0.0)
, max_look(0.0)
, separating(false)
, halting(true)
, seeking(false)
, arriving(false) {
}

Steering::~Steering() {
}

void Steering::Separate(double min_distance, double max_lookaround) noexcept {
	separating = true;
	min_dist = min_distance;
	max_look = max_lookaround;
}

void Steering::DontSeparate() noexcept {
	separating = false;
}

void Steering::ResumeSeparate() noexcept {
	separating = false;
}

void Steering::Halt() noexcept {
	halting = true;
	seeking = false;
	arriving = false;
}

void Steering::Pass(const glm::dvec3 &t) noexcept {
	target = t;
	halting = false;
	seeking = true;
	arriving = false;
}

void Steering::GoTo(const glm::dvec3 &t) noexcept {
	target = t;
	halting = false;
	seeking = false;
	arriving = true;
}

glm::dvec3 Steering::Force(const Situation::State &s) const noexcept {
	double speed = max_speed * glm::clamp(max_speed * haste * haste, 0.25, 1.0);
	double force = max_speed * glm::clamp(max_force * haste * haste, 0.5, 1.0);
	glm::dvec3 result(0.0);
	if (separating) {
		// TODO: off surface situation
		glm::dvec3 repulse(0.0);
		const Situation &s = c.GetSituation();
		for (auto &other : s.GetPlanet().Creatures()) {
			if (&*other == &c) continue;
			glm::dvec3 diff = s.Position() - other->GetSituation().Position();
			if (length2(diff) > max_look * max_look) continue;
			double sep = length(diff) - other->Size() * 0.707 - c.Size() * 0.707;
			if (sep < min_dist) {
				repulse += normalize(diff) * (1.0 - sep / min_dist);
			}
		}
		SumForce(result, repulse, force);
	}
	if (halting) {
		SumForce(result, s.vel * -force, force);
	}
	if (seeking) {
		glm::dvec3 diff = target - s.pos;
		if (!allzero(diff)) {
			SumForce(result, TargetVelocity(s, (normalize(diff) * speed), force), force);
		}
	}
	if (arriving) {
		glm::dvec3 diff = target - s.pos;
		double dist = length(diff);
		if (!allzero(diff) && dist > std::numeric_limits<double>::epsilon()) {
			SumForce(result, TargetVelocity(s, diff * std::min(dist * force, speed) / dist, force), force);
		}
	}
	return result;
}

bool Steering::SumForce(glm::dvec3 &out, const glm::dvec3 &in, double max) const noexcept {
	if (allzero(in) || anynan(in)) {
		return false;
	}
	double cur = allzero(out) ? 0.0 : length(out);
	double rem = max - cur;
	if (rem < 0.0) {
		return true;
	}
	double add = length(in);
	if (add > rem) {
		// this method is off if in and out are in different
		// directions, but gives okayish results
		out += in * (1.0 / add);
		return true;
	} else {
		out += in;
		return false;
	}
}

glm::dvec3 Steering::TargetVelocity(const Situation::State &s, const glm::dvec3 &vel, double acc) const noexcept {
	return (vel - s.vel) * acc;
}

}
}
