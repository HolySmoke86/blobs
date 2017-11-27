#include "Creature.hpp"
#include "Genome.hpp"
#include "Memory.hpp"
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
#include <glm/gtx/transform.hpp>

#include <iostream>
#include <glm/gtx/io.hpp>


namespace blobs {
namespace creature {

Creature::Creature(world::Simulation &sim)
: sim(sim)
, name()
, genome()
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
, steering()
, vel(0.0)
, vao() {
}

Creature::~Creature() {
}

void Creature::Grow(double amount) noexcept {
	Mass(std::min(properties.max_mass, mass + amount));
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

double Creature::Fertility() const noexcept {
	double age = Age();
	if (mass < properties.fertile_mass
		|| age < properties.fertile_age
		|| age > properties.infertile_age) {
		return 0.0;
	}
	return properties.fertility / 3600.0;
}

void Creature::AddGoal(std::unique_ptr<Goal> &&g) {
	std::cout << "[" << int(sim.Time()) << "s] " << name << " new goal: " << g->Describe() << std::endl;
	g->Enable();
	goals.emplace_back(std::move(g));
}

namespace {

bool GoalCompare(const std::unique_ptr<Goal> &a, const std::unique_ptr<Goal> &b) {
	return b->Urgency() < a->Urgency();
}

}

void Creature::Tick(double dt) {
	// TODO: better integration method
	glm::dvec3 acc(steering.Acceleration(*this));
	situation.Move(vel * dt);
	vel += acc * dt;

	if (Age() > properties.death_age) {
		std::cout << "[" << int(sim.Time()) << "s] "
		<< name << " died of old age" << std::endl;
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
		Goal *old_top = &*goals[0];
		std::sort(goals.begin(), goals.end(), GoalCompare);
		Goal *new_top = &*goals[0];
		if (new_top != old_top) {
			std::cout << "[" << int(sim.Time()) << "s] " << name
				<< " changing goal from " << old_top->Describe()
				<< " to " << new_top->Describe() << std::endl;
		}
	}
	goals[0]->Action();
	for (auto goal = goals.begin(); goal != goals.end();) {
		if ((*goal)->Complete()) {
			std::cout << "[" << int(sim.Time()) << "s] " << name
				<< " complete goal: " << (*goal)->Describe() << std::endl;
			goals.erase(goal);
		} else {
			++goal;
		}
	}
}

glm::dmat4 Creature::LocalTransform() noexcept {
	// TODO: surface transform
	const double half_size = size * 0.5;
	const glm::dvec3 &pos = situation.Position();
	return glm::translate(glm::dvec3(pos.x, pos.y, pos.z + half_size))
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
	genome.properties.birth_mass = { 0.5, 0.1 };
	genome.properties.fertile_mass = { 1.0, 0.1 };
	genome.properties.max_mass = { 1.2, 0.1 };
	genome.properties.fertile_age = { 60.0, 5.0 };
	genome.properties.infertile_age = { 700.0, 30.0 };
	genome.properties.death_age = { 900.0, 90.0 };
	genome.properties.fertility = { 0.5, 0.01 };

	if (p.HasAtmosphere()) {
		genome.composition.push_back({
			p.Atmosphere(),    // resource
			{ 0.01, 0.00001 }, // mass
			{ 0.5,  0.001 },   // intake
			{ 0.1,  0.0005 },  // penalty
			{ 0.0,  0.0 },     // growth
		});
	}
	if (liquid > -1) {
		genome.composition.push_back({
			liquid,          // resource
			{ 0.6,  0.01 },  // mass
			{ 0.2,  0.001 }, // intake
			{ 0.01, 0.002 }, // penalty
			{ 0.1, 0.0 },   // growth
		});
	}
	if (solid > -1) {
		genome.composition.push_back({
			solid,             // resource
			{ 0.4,   0.01 },   // mass
			//{ 0.1,   0.001 },  // intake
			{ 0.4,   0.001 },  // intake
			{ 0.001, 0.0001 }, // penalty
			{ 10.0,  0.002 },   // growth
		});
	}

	genome.Configure(c);
}

void Genome::Configure(Creature &c) const {
	c.GetGenome() = *this;

	math::GaloisLFSR &random = c.GetSimulation().Assets().random;

	c.GetProperties().birth_mass = properties.birth_mass.FakeNormal(random.SNorm());
	c.GetProperties().fertile_mass = properties.fertile_mass.FakeNormal(random.SNorm());
	c.GetProperties().max_mass = properties.max_mass.FakeNormal(random.SNorm());
	c.GetProperties().fertile_age = properties.fertile_age.FakeNormal(random.SNorm());
	c.GetProperties().infertile_age = properties.infertile_age.FakeNormal(random.SNorm());
	c.GetProperties().death_age = properties.death_age.FakeNormal(random.SNorm());
	c.GetProperties().fertility = properties.fertility.FakeNormal(random.SNorm());

	double mass = 0.0;
	double volume = 0.0;
	for (const auto &comp : composition) {
		double comp_mass = comp.mass.FakeNormal(random.SNorm());
		double intake = comp.intake.FakeNormal(random.SNorm());
		double penalty = comp.penalty.FakeNormal(random.SNorm());

		mass += comp_mass;
		volume += comp_mass / c.GetSimulation().Resources()[comp.resource].density;

		std::unique_ptr<Need> need;
		if (c.GetSimulation().Resources()[comp.resource].state == world::Resource::SOLID) {
			need.reset(new IngestNeed(comp.resource, intake, penalty));
			need->gain = intake * 0.05;
		} else if (c.GetSimulation().Resources()[comp.resource].state == world::Resource::LIQUID) {
			need.reset(new IngestNeed(comp.resource, intake, penalty));
			need->gain = intake * 0.1;
		} else {
			need.reset(new InhaleNeed(comp.resource, intake, penalty));
			need->gain = intake * 0.5;
		}
		need->name = c.GetSimulation().Resources()[comp.resource].label;
		need->growth = comp.growth.FakeNormal(random.SNorm());
		need->inconvenient = 0.5;
		need->critical = 0.95;
		c.AddNeed(std::move(need));
	}

	c.Mass(c.GetProperties().birth_mass);
	c.Density(mass / volume);
	c.GetSteering().MaxAcceleration(1.4);
	c.GetSteering().MaxSpeed(4.4);
	c.AddGoal(std::unique_ptr<Goal>(new IdleGoal(c)));
}


void Split(Creature &c) {
	Creature *a = new Creature(c.GetSimulation());
	const Situation &s = c.GetSituation();
	// TODO: generate names
	a->Name("Blobby");
	// TODO: mutate
	c.GetGenome().Configure(*a);
	s.GetPlanet().AddCreature(a);
	// TODO: duplicate situation somehow
	a->GetSituation().SetPlanetSurface(
		s.GetPlanet(), s.Surface(),
		s.Position() + glm::dvec3(0.0, a->Size() * 0.51, 0.0));
	a->BuildVAO();

	Creature *b = new Creature(c.GetSimulation());
	b->Name("Sir Blobalot");
	c.GetGenome().Configure(*b);
	s.GetPlanet().AddCreature(b);
	b->GetSituation().SetPlanetSurface(
		s.GetPlanet(), s.Surface(),
		s.Position() + glm::dvec3(0.0, b->Size() * -0.51, 0.0));
	b->BuildVAO();

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


Situation::Situation()
: planet(nullptr)
, position(0.0)
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
	glm::ivec2 t(planet->SurfacePosition(surface, position));
	return type == PLANET_SURFACE
		&& t.x >= 0 && t.x < planet->SideLength()
		&& t.y >= 0 && t.y < planet->SideLength();
}

glm::ivec2 Situation::SurfacePosition() const noexcept {
	return planet->SurfacePosition(surface, position);
}

world::Tile &Situation::GetTile() const noexcept {
	glm::ivec2 t(planet->SurfacePosition(surface, position));
	return planet->TileAt(surface, t.x, t.y);
}

const world::TileType &Situation::GetTileType() const noexcept {
	glm::ivec2 t(planet->SurfacePosition(surface, position));
	return planet->TypeAt(surface, t.x, t.y);
}

void Situation::Move(const glm::dvec3 &dp) noexcept {
	position += dp;
	if (OnSurface()) {
		// enforce ground constraint
		if (Surface() < 3) {
			position[(Surface() + 2) % 3] = std::max(0.0, position[(Surface() + 2) % 3]);
		} else {
			position[(Surface() + 2) % 3] = std::min(0.0, position[(Surface() + 2) % 3]);
		}
	}
}

void Situation::SetPlanetSurface(world::Planet &p, int srf, const glm::dvec3 &pos) noexcept {
	type = PLANET_SURFACE;
	planet = &p;
	surface = srf;
	position = pos;
}


Steering::Steering()
: seek_target(0.0)
, max_accel(1.0)
, max_speed(1.0)
, halting(false)
, seeking(false) {
}

Steering::~Steering() {
}

void Steering::Halt() noexcept {
	halting = true;
	seeking = false;
}

void Steering::GoTo(const glm::dvec3 &t) noexcept {
	seek_target = t;
	halting = false;
	seeking = true;
}

glm::dvec3 Steering::Acceleration(Creature &c) const noexcept {
	glm::dvec3 acc(0.0);
	if (halting) {
		SumForce(acc, c.Velocity() * -max_accel);
	}
	if (seeking) {
		glm::dvec3 diff = seek_target - c.GetSituation().Position();
		if (!allzero(diff)) {
			SumForce(acc, ((normalize(diff) * max_speed) - c.Velocity()) * max_accel);
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

}
}
