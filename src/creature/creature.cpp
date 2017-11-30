#include "Creature.hpp"
#include "Genome.hpp"
#include "Memory.hpp"
#include "NameGenerator.hpp"
#include "Situation.hpp"
#include "Steering.hpp"

#include "Goal.hpp"
#include "IdleGoal.hpp"
#include "InhaleNeed.hpp"
#include "IngestNeed.hpp"
#include "Need.hpp"
#include "../app/Assets.hpp"
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

Creature::Creature(world::Simulation &sim)
: sim(sim)
, name()
, genome()
, properties()
, cur_prop(0)
, base_color(1.0)
, highlight_color(0.0)
, mass(1.0)
, density(1.0)
, size(1.0)
, birth(sim.Time())
, health(1.0)
, on_death()
, removable(false)
, memory(*this)
, needs()
, goals()
, situation()
, steering(*this)
, vao() {
	// all creatures avoid each other for now
	steering.Separate(0.1, 1.5);
}

Creature::~Creature() {
}

glm::dvec4 Creature::HighlightColor() const noexcept {
	return glm::dvec4(highlight_color, AgeLerp(CurProps().highlight, NextProps().highlight));
}

void Creature::Ingest(int res, double amount) noexcept {
	const Genome::Composition *cmp = nullptr;
	for (const auto &c : genome.composition) {
		if (c.resource == res) {
			cmp = &c;
			break;
		}
	}
	if (cmp) {
		const double max_mass = AgeLerp(CurProps().mass, NextProps().mass);
		Mass(std::min(max_mass, mass + amount));
	} else {
		// foreign material. poisonous?
	}
}

void Creature::Hurt(double dt) noexcept {
	health = std::max(0.0, health - dt);
	if (health == 0.0) {
		std::cout << "[" << int(sim.Time()) << "s] "
			<< name << " died" << std::endl;
		Die();
	}
}

void Creature::Die() noexcept {
	needs.clear();
	goals.clear();
	steering.Halt();
	if (on_death) {
		on_death(*this);
	}
	Remove();
}

double Creature::Size() const noexcept {
	return size;
}

double Creature::Age() const noexcept {
	return sim.Time() - birth;
}

std::string Creature::AgeName() const {
	switch (cur_prop) {
		case 0:
			return "Newborn";
		case 1:
			return "Child";
		case 2:
			return "Youth";
		case 3:
			return "Adult";
		case 4:
			return "Elder";
		case 5:
			return "Dead";
		default:
			return "Unknown";
	}
}

double Creature::AgeLerp(double from, double to) const noexcept {
	return glm::mix(from, to, glm::smoothstep(CurProps().age, NextProps().age, Age()));
}

double Creature::Fertility() const noexcept {
	return AgeLerp(CurProps().fertility, NextProps().fertility) / 3600.0;
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
	if (cur_prop < 5 && Age() > NextProps().age) {
		++cur_prop;
		if (cur_prop == 5) {
			std::cout << "[" << int(sim.Time()) << "s] "
				<< name << " died of old age" << std::endl;
			Die();
		} else {
			std::cout << "[" << int(sim.Time()) << "s] "
				<< name << " grew up to " << AgeName() << std::endl;
		}
	}

	{
		Situation::State state(situation.GetState());
		Situation::Derivative a(Step(Situation::Derivative(), 0.0));
		Situation::Derivative b(Step(a, dt * 0.5));
		Situation::Derivative c(Step(b, dt * 0.5));
		Situation::Derivative d(Step(c, dt));
		Situation::Derivative f(
			(1.0 / 6.0) * (a.vel + 2.0 * (b.vel + c.vel) + d.vel),
			(1.0 / 6.0) * (a.acc + 2.0 * (b.acc + c.acc) + d.acc),
			(1.0 / 6.0) * (a.turn + 2.0 * (b.turn + c.turn) + d.turn)
		);
		state.pos += f.vel * dt;
		state.vel += f.acc * dt;
		constexpr double turn_speed = 10.0;
		// TODO: this is crap
		state.dir = glm::normalize(state.dir + f.turn * turn_speed * dt);
		situation.SetState(state);
	}

	memory.Tick(dt);
	for (auto &need : needs) {
		need->Tick(dt);
	}
	for (auto &goal : goals) {
		goal->Tick(dt);
	}
	// do background stuff
	for (auto &need : needs) {
		need->ApplyEffect(*this, dt);
	}
	if (goals.empty()) {
		return;
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

Situation::Derivative Creature::Step(const Situation::Derivative &ds, double dt) const noexcept {
	Situation::State s = situation.GetState();
	s.pos += ds.vel * dt;
	s.vel += ds.acc * dt;
	s.dir = normalize(s.dir + ds.turn * dt);
	return {
		s.vel,
		steering.Acceleration(s),
		allzero(s.vel) ? glm::dvec3(0.0) : normalize(s.vel) - s.dir
	};
}

glm::dmat4 Creature::LocalTransform() noexcept {
	// TODO: surface transform
	const double half_size = size * 0.5;
	const glm::dvec3 &pos = situation.Position();
	return glm::translate(glm::dvec3(pos.x, pos.y, pos.z + half_size))
		* glm::dmat4(world::Planet::SurfaceOrientation(situation.Surface()))
		* glm::rotate(glm::orientedAngle(glm::dvec3(0.0, 0.0, -1.0), situation.Heading(), glm::dvec3(0.0, 1.0, 0.0)), glm::dvec3(0.0, 1.0, 0.0))
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

	genome.properties.Birth().age = { 0.0, 0.0 };
	genome.properties.Birth().mass = { 0.5, 0.05 };
	genome.properties.Birth().fertility = { 0.0, 0.0 };
	genome.properties.Birth().highlight = { 0.0, 0.0 };

	genome.properties.Child().age = { 30.0, 1.0 };
	genome.properties.Child().mass = { 0.7, 0.05 };
	genome.properties.Child().fertility = { 0.0, 0.0 };
	genome.properties.Child().highlight = { 0.2, 0.05 };

	genome.properties.Youth().age = { 60.0, 5.0 };
	genome.properties.Youth().mass = { 0.9, 0.1 };
	genome.properties.Youth().fertility = { 0.5, 0.03 };
	genome.properties.Youth().highlight = { 0.9, 0.1 };

	genome.properties.Adult().age = { 120.0, 10.0 };
	genome.properties.Adult().mass = { 1.2, 0.1 };
	genome.properties.Adult().fertility = { 0.4, 0.01 };
	genome.properties.Adult().highlight = { 0.7, 0.1 };

	genome.properties.Elder().age = { 360.0, 30.0 };
	genome.properties.Elder().mass = { 1.0, 0.05 };
	genome.properties.Elder().fertility = { 0.1, 0.01 };
	genome.properties.Elder().highlight = { 0.6, 0.1 };

	genome.properties.Death().age = { 480.0, 60.0 };
	genome.properties.Death().mass = { 0.9, 0.05 };
	genome.properties.Death().fertility = { 0.0, 0.0 };
	genome.properties.Death().highlight = { 0.5, 0.1 };

	genome.properties.strength = { 1.0, 0.1 };
	genome.properties.stamina = { 1.0, 0.1 };
	genome.properties.dexerty = { 1.0, 0.1 };
	genome.properties.intelligence = { 1.0, 0.1 };
	genome.properties.mutability = { 1.0, 0.1 };

	glm::dvec3 color_avg(0.0);
	double color_divisor = 0.0;

	if (p.HasAtmosphere()) {
		genome.composition.push_back({
			p.Atmosphere(),    // resource
			{ 0.01, 0.00001 }, // mass
			{ 0.5,  0.001 },   // intake
			{ 0.1,  0.0005 },  // penalty
			{ 0.0,  0.0 },     // growth
		});
		color_avg += c.GetSimulation().Resources()[p.Atmosphere()].base_color * 0.1;
		color_divisor += 0.1;
	}
	if (liquid > -1) {
		genome.composition.push_back({
			liquid,          // resource
			{ 0.6,  0.01 },  // mass
			{ 0.2,  0.001 }, // intake
			{ 0.01, 0.002 }, // penalty
			{ 0.1, 0.0 },   // growth
		});
		color_avg += c.GetSimulation().Resources()[liquid].base_color * 0.5;
		color_divisor += 0.5;
	}
	if (solid > -1) {
		genome.composition.push_back({
			solid,             // resource
			{ 0.4,   0.01 },   // mass
			{ 0.4,   0.001 },  // intake
			{ 0.001, 0.0001 }, // penalty
			{ 10.0,  0.002 },   // growth
		});
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

	double mass = 0.0;
	double volume = 0.0;
	for (const auto &comp : composition) {
		const world::Resource &resource = c.GetSimulation().Resources()[comp.resource];
		double comp_mass = comp.mass.FakeNormal(random.SNorm());
		double intake = comp.intake.FakeNormal(random.SNorm());
		double penalty = comp.penalty.FakeNormal(random.SNorm());

		mass += comp_mass;
		volume += comp_mass / c.GetSimulation().Resources()[comp.resource].density;

		std::unique_ptr<Need> need;
		if (resource.state == world::Resource::SOLID) {
			intake *= std::atan(c.GetProperties().strength);
			need.reset(new IngestNeed(comp.resource, intake, penalty));
			need->gain = intake * 0.05;
		} else if (resource.state == world::Resource::LIQUID) {
			intake *= std::atan(c.GetProperties().stamina);
			need.reset(new IngestNeed(comp.resource, intake, penalty));
			need->gain = intake * 0.1;
		} else {
			need.reset(new InhaleNeed(comp.resource, intake, penalty));
			need->gain = intake * 0.5;
		}
		need->name = c.GetSimulation().Resources()[comp.resource].label;
		need->value = 0.4;
		need->inconvenient = 0.5;
		need->critical = 0.95;
		c.AddNeed(std::move(need));
	}

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

	c.Mass(c.GetProperties().props[0].mass);
	c.Density(mass / volume);
	c.GetSteering().MaxAcceleration(1.4 * std::atan(c.GetProperties().strength));
	c.GetSteering().MaxSpeed(4.4 * std::atan(c.GetProperties().dexerty));
	c.AddGoal(std::unique_ptr<Goal>(new IdleGoal(c)));
}


void Split(Creature &c) {
	Creature *a = new Creature(c.GetSimulation());
	const Situation &s = c.GetSituation();
	a->Name(c.GetSimulation().Assets().name.Sequential());
	// TODO: mutate
	c.GetGenome().Configure(*a);
	s.GetPlanet().AddCreature(a);
	// TODO: duplicate situation somehow
	a->GetSituation().SetPlanetSurface(
		s.GetPlanet(), s.Surface(),
		s.Position() + glm::dvec3(0.0, a->Size() * 0.51, 0.0));
	a->BuildVAO();
	std::cout << "[" << int(c.GetSimulation().Time()) << "s] "
		<< a->Name() << " was born" << std::endl;

	Creature *b = new Creature(c.GetSimulation());
	b->Name(c.GetSimulation().Assets().name.Sequential());
	c.GetGenome().Configure(*b);
	s.GetPlanet().AddCreature(b);
	b->GetSituation().SetPlanetSurface(
		s.GetPlanet(), s.Surface(),
		s.Position() + glm::dvec3(0.0, b->Size() * -0.51, 0.0));
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
	if (s.OnSurface()) {
		TrackStay({ &s.GetPlanet(), s.Surface(), s.SurfacePosition() }, dt);
	}
}

void Memory::TrackStay(const Location &l, double t) {
	const world::TileType &type = l.planet->TypeAt(l.surface, l.coords.x, l.coords.y);
	auto entry = known_types.find(type.id);
	if (entry != known_types.end()) {
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
, max_accel(1.0)
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

glm::dvec3 Steering::Acceleration(const Situation::State &s) const noexcept {
	glm::dvec3 acc(0.0);
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
		SumForce(acc, repulse);
	}
	if (halting) {
		SumForce(acc, s.vel * -max_accel);
	}
	if (seeking) {
		glm::dvec3 diff = target - s.pos;
		if (!allzero(diff)) {
			SumForce(acc, TargetVelocity(s, (normalize(diff) * max_speed)));
		}
	}
	if (arriving) {
		glm::dvec3 diff = target - s.pos;
		double dist = length(diff);
		if (!allzero(diff) && dist > std::numeric_limits<double>::epsilon()) {
			SumForce(acc, TargetVelocity(s, diff * std::min(dist * max_accel, max_speed) / dist));
		}
	}
	return acc;
}

bool Steering::SumForce(glm::dvec3 &out, const glm::dvec3 &in) const noexcept {
	if (allzero(in) || anynan(in)) {
		return false;
	}
	double cur = allzero(out) ? 0.0 : length(out);
	double rem = max_accel - cur;
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

glm::dvec3 Steering::TargetVelocity(const Situation::State &s, const glm::dvec3 &vel) const noexcept {
	return (vel - s.vel) * max_accel;
}

}
}
