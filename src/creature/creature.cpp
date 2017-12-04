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
#include "../ui/string.hpp"
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
: components()
, total_mass(0.0) {
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
	for (auto c = components.begin(); c != components.end(); ++c) {
		if (c->resource == res) {
			c->value += amount;
			if (c->value <= 0.0) {
				components.erase(c);
			}
			found = true;
			break;
		}
	}
	if (!found && amount > 0.0) {
		components.emplace_back(res, amount);
	}
	std::sort(components.begin(), components.end(), CompositionCompare);
	total_mass += amount;
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
, death(-1.0)
, on_death()
, removable(false)
, parents()
, stats()
, memory(*this)
, bg_task()
, goals()
, situation()
, steering(*this)
, vao() {
	sim.SetAlive(this);
	// all creatures avoid each other for now
	steering.Separate(0.1, 1.5);
}

Creature::~Creature() {
}

void Creature::AddMass(int res, double amount) {
	composition.Add(res, amount);
	double nonsolid = 0.0;
	double volume = 0.0;
	for (const auto &c : composition) {
		volume += c.value / sim.Assets().data.resources[c.resource].density;
		if (sim.Assets().data.resources[c.resource].state != world::Resource::SOLID) {
			nonsolid += c.value;
		}
	}
	Mass(composition.TotalMass());
	Size(std::cbrt(volume));
	highlight_color.a = nonsolid / composition.TotalMass();
}

void Creature::HighlightColor(const glm::dvec3 &c) noexcept {
	highlight_color = glm::dvec4(c, highlight_color.a);
}

void Creature::Ingest(int res, double amount) noexcept {
	// TODO: check foreign materials
	if (sim.Resources()[res].state == world::Resource::SOLID) {
		// 15% of solids stays in body
		AddMass(res, amount * 0.15);
	} else {
		// 10% of fluids stays in body
		AddMass(res, amount * 0.05);
	}
	math::GaloisLFSR &random = sim.Assets().random;
	if (random.UNorm() < AdaptChance()) {
		// change color to be slightly more like resource
		glm::dvec3 color(rgb2hsl(sim.Resources()[res].base_color));
		// solids affect base color, others highlight
		double p = sim.Resources()[res].state == world::Resource::SOLID ? 0 : 1;
		double q = random.UInt(3); // hue, sat, or val
		double r = random.UInt(2); // mean or deviation
		math::Distribution *d = nullptr;
		double ref = 0.0;
		if (p == 0) {
			if (q == 0) {
				d = &genome.base_hue;
				ref = color.x;
			} else if (q == 1) {
				d = &genome.base_saturation;
				ref = color.y;
			} else {
				d = &genome.base_lightness;
				ref = color.z;
			}
		} else {
			if (q == 0) {
				d = &genome.highlight_hue;
				ref = color.x;
			} else if (q == 1) {
				d = &genome.highlight_saturation;
				ref = color.y;
			} else {
				d = &genome.highlight_lightness;
				ref = color.z;
			}
		}
		if (r == 0) {
			double diff = ref - d->Mean();
			if (q == 0) {
				if (diff < -0.5) {
					diff += 1.0;
				} else if (diff > 0.5) {
					diff -= 1.0;
				}
				// move ±15% of distance
				d->Mean(std::fmod(d->Mean() + diff * random.SNorm() * 0.15, 1.0));
			} else {
				d->Mean(glm::clamp(d->Mean() + diff * random.SNorm() * 0.15, 0.0, 1.0));
			}
		} else {
			// scale by ±15%, enforce bounds
			d->StandardDeviation(glm::clamp(d->StandardDeviation() * (1.0 + random.SNorm() * 0.15), 0.0001, 0.5));
		}
	}
}

void Creature::DoWork(double amount) noexcept {
	stats.Exhaustion().Add(amount / Stamina());
	// burn resources proportional to composition
	// factor = 1/total * 1/efficiency * amount * -1
	double factor = -amount / (composition.TotalMass() * EnergyEfficiency());
	// make a copy to total remains constant and
	// no entries disappear during iteration
	Composition comp(composition);
	for (auto &cmp : comp) {
		double value = cmp.value * factor * sim.Resources()[cmp.resource].inverse_energy;
		AddMass(cmp.resource, value);
	}
}

void Creature::Hurt(double amount) noexcept {
	stats.Damage().Add(amount);
	if (stats.Damage().Full()) {
		Die();
	}
}

void Creature::Die() noexcept {
	if (Dead()) return;

	std::ostream &log = sim.Log() << name << " ";
	if (stats.Exhaustion().Full()) {
		log << "died of exhaustion";
	} else if (stats.Breath().Full()) {
		log << "suffocated";
	} else if (stats.Thirst().Full()) {
		log << "died of thirst";
	} else if (stats.Hunger().Full()) {
		log << "starved to death";
	} else {
		log << "succumed to wounds";
	}
	log << " at an age of " << ui::TimeString(Age())
		<< " (" << ui::PercentageString(Age() / properties.Lifetime())
		<< " of life expectancy of " << ui::TimeString(properties.Lifetime())
		<< ")" << std::endl;

	sim.SetDead(this);
	death = sim.Time();
	steering.Off();
	if (on_death) {
		on_death(*this);
	}
	Remove();
}

bool Creature::Dead() const noexcept {
	return death > birth;
}

void Creature::Remove() noexcept {
	removable = true;
}

void Creature::Removed() noexcept {
	bg_task.reset();
	goals.clear();
	memory.Erase();
	KillVAO();
}

void Creature::AddParent(Creature &p) {
	parents.push_back(&p);
}

double Creature::Age() const noexcept {
	return sim.Time() - birth;
}

double Creature::AgeFactor(double peak) const noexcept {
	// shifted inverse hermite, y = 1 - (3t² - 2t³) with t = normalized age - peak
	// goes negative below -0.5 and starts to rise again above 1.0
	double t = glm::clamp((Age() / properties.Lifetime()) - peak, -0.5, 1.0);
	// guarantee at least 1%
	return std::max(0.01, 1.0 - (3.0 * t * t) + (2.0 * t * t * t));
}

double Creature::EnergyEfficiency() const noexcept {
	return 0.25 * AgeFactor(0.05);
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

double Creature::Adaptability() const noexcept {
	return properties.Adaptability();
}

double Creature::OffspringMass() const noexcept {
	return properties.OffspringMass();
}

double Creature::PerceptionRange() const noexcept {
	return 3.0 * (Dexerty() / (Dexerty() + 1)) + Size();
}

double Creature::PerceptionOmniRange() const noexcept {
	return 0.5 * (Dexerty() / (Dexerty() + 1)) + Size();
}

double Creature::PerceptionField() const noexcept {
	// this is the cosine of half the angle, so 1.0 is none, -1.0 is perfect
	return 0.8 - (Dexerty() / (Dexerty() + 1));
}

bool Creature::PerceptionTest(const glm::dvec3 &p) const noexcept {
	const glm::dvec3 diff(p - situation.Position());
	double omni_range = PerceptionOmniRange();
	if (length2(diff) < omni_range * omni_range) return true;
	double range = PerceptionRange();
	if (length2(diff) > range * range) return false;
	return dot(normalize(diff), situation.Heading()) > PerceptionField();
}

double Creature::OffspringChance() const noexcept {
	return AgeFactor(0.25) * properties.Fertility() * (1.0 / 3600.0);
}

double Creature::MutateChance() const noexcept {
	return GetProperties().Mutability() * (1.0 / 3600.0);
}

double Creature::AdaptChance() const noexcept {
	return GetProperties().Adaptability() * (1.0 / 120.0);
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
	situation.EnforceConstraints(state);
	if (length2(state.vel) > 0.000001) {
		glm::dvec3 nvel(normalize(state.vel));
		double ang = angle(nvel, state.dir);
		double turn_rate = PI * 0.75 * dt;
		if (ang < turn_rate) {
			state.dir = normalize(state.vel);
		} else if (std::abs(ang - PI) < 0.001) {
			state.dir = rotate(state.dir, turn_rate, world::Planet::SurfaceNormal(situation.Surface()));
		} else {
			state.dir = rotate(state.dir, turn_rate, normalize(cross(state.dir, nvel)));
		}
	}
	situation.SetState(state);
	// work is force times distance
	DoWork(length(f.acc) * Mass() * length(f.vel) * dt);
}

Situation::Derivative Creature::Step(const Situation::Derivative &ds, double dt) const noexcept {
	Situation::State s = situation.GetState();
	s.pos += ds.vel * dt;
	s.vel += ds.acc * dt;
	glm::dvec3 force(steering.Force(s));
	// gravity = antinormal * mass * Gm / r²
	double elevation = s.pos[(situation.Surface() + 2) % 3];
	glm::dvec3 normal(world::Planet::SurfaceNormal(situation.Surface()));
	force += glm::dvec3(
		-normal
		* Mass() * situation.GetPlanet().GravitationalParameter()
		/ (elevation * elevation));
	// if net force is applied and in contact with surface
	if (!allzero(force) && std::abs(std::abs(elevation) - situation.GetPlanet().Radius()) < 0.001) {
		// apply friction = -|normal force| * tangential force * coefficient
		glm::dvec3 fn(normal * dot(force, normal));
		glm::dvec3 ft(force - fn);
		double u = 0.4;
		glm::dvec3 friction(-length(fn) * ft * u);
		force += friction;
	}
	return {
		s.vel,
		force / Mass()
	};
}

void Creature::TickStats(double dt) {
	for (auto &s : stats.stat) {
		s.Add(s.gain * dt);
	}
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
	if (!situation.Moving()) {
		// double exhaustion recovery when standing still
		stats.Exhaustion().Add(stats.Exhaustion().gain * dt);
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

math::AABB Creature::CollisionBox() const noexcept {
	return { glm::dvec3(size * -0.5), glm::dvec3(size * 0.5) };
}

glm::dmat4 Creature::CollisionTransform() const noexcept {
	const double half_size = size * 0.5;
	const glm::dvec3 &pos = situation.Position();
	const glm::dmat3 srf(world::Planet::SurfaceOrientation(situation.Surface()));
	return glm::translate(glm::dvec3(pos.x, pos.y, pos.z + half_size))
		* glm::rotate(glm::orientedAngle(-srf[2], situation.Heading(), srf[1]), srf[1])
		* glm::dmat4(srf);
}

glm::dmat4 Creature::LocalTransform() noexcept {
	const double half_size = size * 0.5;
	return CollisionTransform()
		* glm::scale(glm::dvec3(half_size, half_size, half_size));
}

void Creature::BuildVAO() {
	vao.reset(new graphics::SimpleVAO<Attributes, unsigned short>);
	vao->Bind();
	vao->BindAttributes();
	vao->EnableAttribute(0);
	vao->EnableAttribute(1);
	vao->EnableAttribute(2);
	vao->AttributePointer<glm::vec3>(0, false, offsetof(Attributes, position));
	vao->AttributePointer<glm::vec3>(1, false, offsetof(Attributes, normal));
	vao->AttributePointer<glm::vec3>(2, false, offsetof(Attributes, texture));
	vao->ReserveAttributes(6 * 4, GL_STATIC_DRAW);
	{
		auto attrib = vao->MapAttributes(GL_WRITE_ONLY);
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
	vao->BindElements();
	vao->ReserveElements(6 * 6, GL_STATIC_DRAW);
	{
		auto element = vao->MapElements(GL_WRITE_ONLY);
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
	vao->Unbind();
}

void Creature::KillVAO() {
	vao.reset();
}

void Creature::Draw(graphics::Viewport &viewport) {
	if (!vao) return;
	vao->Bind();
	vao->DrawTriangles(6 * 6);
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
	genome.properties.Stamina() = { 2.0, 0.1 };
	genome.properties.Dexerty() = { 2.0, 0.1 };
	genome.properties.Intelligence() = { 1.0, 0.1 };
	genome.properties.Lifetime() = { 480.0, 60.0 };
	genome.properties.Fertility() = { 0.5, 0.03 };
	genome.properties.Mutability() = { 0.9, 0.1 };
	genome.properties.Adaptability() = { 0.9, 0.1 };
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
	// use opposite color as start highlight
	genome.highlight_hue = { std::fmod(hsl.x + 0.5, 1.0), 0.01 };
	genome.highlight_saturation = { 1.0 - hsl.y, 0.01 };
	genome.highlight_lightness = { 1.0 - hsl.z, 0.01 };

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
		std::fmod(highlight_hue.FakeNormal(random.SNorm()) + 1.0, 1.0),
		glm::clamp(highlight_saturation.FakeNormal(random.SNorm()), 0.0, 1.0),
		glm::clamp(highlight_lightness.FakeNormal(random.SNorm()), 0.0, 1.0)
	);
	c.BaseColor(hsl2rgb(base_color));
	c.HighlightColor(hsl2rgb(highlight_color));
	c.SetBackgroundTask(std::unique_ptr<Goal>(new BlobBackgroundTask(c)));
	c.AddGoal(std::unique_ptr<Goal>(new IdleGoal(c)));
}


void Split(Creature &c) {
	Creature *a = new Creature(c.GetSimulation());
	const Situation &s = c.GetSituation();
	a->AddParent(c);
	a->Name(c.GetSimulation().Assets().name.Sequential());
	c.GetGenome().Configure(*a);
	for (const auto &cmp : c.GetComposition()) {
		a->AddMass(cmp.resource, cmp.value * 0.5);
	}
	s.GetPlanet().AddCreature(a);
	// TODO: duplicate situation somehow
	a->GetSituation().SetPlanetSurface(
		s.GetPlanet(), s.Surface(),
		s.Position() + glm::dvec3(0.0, 0.55 * a->Size(), 0.0));
	a->BuildVAO();
	c.GetSimulation().Log() << a->Name() << " was born" << std::endl;

	Creature *b = new Creature(c.GetSimulation());
	b->AddParent(c);
	b->Name(c.GetSimulation().Assets().name.Sequential());
	c.GetGenome().Configure(*b);
	for (const auto &cmp : c.GetComposition()) {
		b->AddMass(cmp.resource, cmp.value * 0.5);
	}
	s.GetPlanet().AddCreature(b);
	b->GetSituation().SetPlanetSurface(
		s.GetPlanet(), s.Surface(),
		s.Position() - glm::dvec3(0.0, 0.55 * b->Size(), 0.0));
	b->BuildVAO();
	c.GetSimulation().Log() << b->Name() << " was born" << std::endl;

	c.Die();
}


Memory::Memory(Creature &c)
: c(c) {
}

Memory::~Memory() {
}

void Memory::Erase() {
	known_types.clear();
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
	if (type != PLANET_SURFACE) return false;
	glm::ivec2 t(planet->SurfacePosition(surface, state.pos));
	return t.x >= 0 && t.x < planet->SideLength()
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
	EnforceConstraints(state);
}

void Situation::Accelerate(const glm::dvec3 &dv) noexcept {
	state.vel += dv;
	EnforceConstraints(state);
}

void Situation::EnforceConstraints(State &s) noexcept {
	if (OnSurface()) {
		if (Surface() < 3) {
			if (s.pos[(Surface() + 2) % 3] < GetPlanet().Radius()) {
				s.pos[(Surface() + 2) % 3] = GetPlanet().Radius();
				s.vel[(Surface() + 2) % 3] = std::max(0.0, s.vel[(Surface() + 2) % 3]);
			}
		} else {
			if (s.pos[(Surface() + 2) % 3] > -GetPlanet().Radius()) {
				s.pos[(Surface() + 2) % 3] = -GetPlanet().Radius();
				s.vel[(Surface() + 2) % 3] = std::min(0.0, s.vel[(Surface() + 2) % 3]);
			}
		}
	}
}

void Situation::SetPlanetSurface(world::Planet &p, int srf, const glm::dvec3 &pos) noexcept {
	type = PLANET_SURFACE;
	planet = &p;
	surface = srf;
	state.pos = pos;
	EnforceConstraints(state);
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

void Steering::Off() noexcept {
	separating = false;
	halting = false;
	seeking = false;
	arriving = false;
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
	separating = true;
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
			if (!c.PerceptionTest(other->GetSituation().Position())) continue;
			double sep = length(diff) - other->Size() * 0.707 - c.Size() * 0.707;
			if (sep < min_dist) {
				repulse += normalize(diff) * (1.0 - sep / min_dist);
			}
		}
		result += repulse;
	}
	if (halting) {
		// break twice as hard
		result += -2.0 * s.vel * force;
	}
	if (seeking) {
		glm::dvec3 diff = target - s.pos;
		if (!allzero(diff)) {
			result += TargetVelocity(s, (normalize(diff) * speed), force);
		}
	}
	if (arriving) {
		glm::dvec3 diff = target - s.pos;
		double dist = length(diff);
		if (!allzero(diff) && dist > std::numeric_limits<double>::epsilon()) {
			result += TargetVelocity(s, diff * std::min(dist * force, speed) / dist, force);
		}
	}
	if (length2(result) > max_force * max_force) {
		result = normalize(result) * max_force;
	}
	return result;
}

glm::dvec3 Steering::TargetVelocity(const Situation::State &s, const glm::dvec3 &vel, double acc) const noexcept {
	return (vel - s.vel) * acc;
}

}
}
