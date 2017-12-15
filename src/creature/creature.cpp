#include "Composition.hpp"
#include "Creature.hpp"
#include "Genome.hpp"
#include "Memory.hpp"
#include "NameGenerator.hpp"
#include "Situation.hpp"
#include "Steering.hpp"

#include "AttackGoal.hpp"
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

Composition::Composition(const world::Set<world::Resource> &resources)
: resources(resources)
, components()
, total_mass(0.0)
, total_volume(0.0)
, state_mass{0.0} {
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
				amount += c->value;
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
	state_mass[resources[res].state] += amount;
	total_mass += amount;
	total_volume += amount / resources[res].density;
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

double Composition::Proportion(int res) const noexcept {
	return Get(res) / TotalMass();
}

double Composition::StateProportion(int res) const noexcept {
	return Get(res) / StateMass(resources[res].state);
}

double Composition::Compatibility(int res) const noexcept {
	if (Has(res)) {
		return StateProportion(res);
	}
	double max_compat = -1.0;
	double min_compat = 1.0;
	for (const auto &c : components) {
		double prop = c.value / StateMass(resources[res].state);
		for (const auto &compat : resources[c.resource].compatibility) {
			double value = compat.second * prop;
			if (value > max_compat) {
				max_compat = value;
			}
			if (value < min_compat) {
				min_compat = value;
			}
		}
	}
	if (min_compat < 0.0) {
		return min_compat;
	} else {
		return max_compat;
	}
}


Creature::Creature(world::Simulation &sim)
: sim(sim)
, name()
, genome()
, properties()
, composition(sim.Resources())
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
, heading_target(0.0, 0.0, -1.0)
, heading_manual(false)
, perception_range(1.0)
, perception_range_squared(1.0)
, perception_omni_range(1.0)
, perception_omni_range_squared(1.0)
, perception_field(1.0)
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
		volume += c.value / sim.Resources()[c.resource].density;
		if (sim.Resources()[c.resource].state != world::Resource::SOLID) {
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
	if (sim.Resources()[res].state == world::Resource::SOLID) {
		// 30% of solids stays in body
		AddMass(res, amount * 0.3 * composition.Compatibility(res));
	} else {
		// 5% of fluids stays in body
		AddMass(res, amount * 0.05 * composition.Compatibility(res));
	}
	math::GaloisLFSR &random = sim.Assets().random;
	if (random.UNorm() < AdaptChance()) {
		// change color to be slightly more like resource
		glm::dvec3 color(rgb2hsl(sim.Resources()[res].base_color));
		// solids affect base color, others highlight
		int p = sim.Resources()[res].state == world::Resource::SOLID ? 0 : 1;
		int q = random.UInt(3); // hue, sat, or val
		int r = random.UInt(2); // mean or deviation
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
				// move 0-15% of distance
				d->Mean(std::fmod(d->Mean() + diff * random.UNorm() * 0.15, 1.0));
			} else {
				d->Mean(glm::clamp(d->Mean() + diff * random.UNorm() * 0.15, 0.0, 1.0));
			}
		} else {
			// scale by ±15%, enforce bounds
			d->StandardDeviation(glm::clamp(d->StandardDeviation() * (1.0 + random.SNorm() * 0.15), 0.0001, 0.5));
		}
	}
	if (sim.Resources()[res].state == world::Resource::LIQUID && random.UNorm() < AdaptChance()) {
		// change texture randomly
		// TODO: make change depending on surroundings and/or resource
		int p = random.UInt(2); // back or side
		int q = random.UInt(2); // mean or deviation
		math::Distribution &d = p ? genome.skin_side : genome.skin_back;
		if (q == 0) {
			// move ± one standard deviation
			d.Mean(d.Mean() + (random.SNorm() * d.StandardDeviation()));
		} else {
			// scale by ±10%, enforce bounds
			d.StandardDeviation(glm::clamp(d.StandardDeviation() * (1.0 + random.SNorm() * 0.1), 0.0001, 0.5));
		}
	}
}

void Creature::DoWork(double amount) noexcept {
	stats.Exhaustion().Add(amount / (Stamina() + 1.0));
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
	// doing work improves strength a little
	properties.Strength() += amount * 0.0001;
}

void Creature::Hurt(double amount) noexcept {
	stats.Damage().Add(amount);
	if (stats.Damage().Full()) {
		Die();
	}
}

void Creature::Die() noexcept {
	if (Dead()) return;

	if (stats.Damage().Full()) {
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
	}

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
	return Dead() ? death - birth : sim.Time() - birth;
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

double Creature::StrengthFactor() const noexcept {
	double str = Strength();
	return str / (str + 1.0);
}

double Creature::Stamina() const noexcept {
	return properties.Stamina() * ExhaustionFactor() * AgeFactor(0.25);
}

double Creature::StaminaFactor() const noexcept {
	double stm = Stamina();
	return stm / (stm + 1.0);
}

double Creature::Dexerty() const noexcept {
	return properties.Dexerty() * ExhaustionFactor() * AgeFactor(0.25);
}

double Creature::DexertyFactor() const noexcept {
	double dex = Dexerty();
	return dex / (dex + 1.0);
}

double Creature::Intelligence() const noexcept {
	return properties.Intelligence() * FatigueFactor() * AgeFactor(0.25);
}

double Creature::IntelligenceFactor() const noexcept {
	double intl = Intelligence();
	return intl / (intl + 1.0);
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
	return perception_range;
}

double Creature::PerceptionOmniRange() const noexcept {
	return perception_omni_range;
}

double Creature::PerceptionField() const noexcept {
	return perception_field;
}

bool Creature::PerceptionTest(const glm::dvec3 &p) const noexcept {
	const glm::dvec3 diff(p - situation.Position());
	double ldiff = glm::length2(diff);
	if (ldiff < perception_omni_range_squared) return true;
	if (ldiff > perception_range_squared) return false;
	return glm::dot(diff / std::sqrt(ldiff), situation.Heading()) > perception_field;
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
	if (goals.empty()) {
		g->SetForeground();
	}
	goals.emplace_back(std::move(g));
}

void Creature::SetBackgroundTask(std::unique_ptr<Goal> &&g) {
	bg_task = std::move(g);
}

Goal &Creature::BackgroundTask() {
	return *bg_task;
}

namespace {

bool GoalCompare(const std::unique_ptr<Goal> &a, const std::unique_ptr<Goal> &b) {
	return b->Urgency() < a->Urgency();
}

}

void Creature::Tick(double dt) {
	Cache();
	TickState(dt);
	TickStats(dt);
	TickBrain(dt);
}

void Creature::Cache() noexcept {
	double dex_fact = DexertyFactor();
	perception_range = 3.0 * dex_fact + size;
	perception_range_squared = perception_range * perception_range;
	perception_omni_range = 0.5 * dex_fact + size;
	perception_omni_range_squared = perception_omni_range * perception_omni_range;
	// this is the cosine of half the angle, so 1.0 is none, -1.0 is perfect
	perception_field = 0.8 - dex_fact;
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

	if (!heading_manual && glm::length2(state.vel) > 0.000001) {
		const glm::dvec3 normal(situation.GetPlanet().NormalAt(state.pos));
		const glm::dvec3 tangent(state.vel - (normal * glm::dot(state.vel, normal)));
		if (glm::length2(tangent) > 0.000001) {
			heading_target = glm::normalize(tangent);
		}
	}
	double ang = glm::angle(heading_target, state.dir);
	double turn_rate = PI * 0.75 * dt;
	if (ang < turn_rate) {
		state.dir = heading_target;
		heading_manual = false;
	} else {
		state.dir = glm::rotate(state.dir, turn_rate, glm::normalize(glm::cross(state.dir, heading_target)));
	}

	situation.SetState(state);
	// work is force times distance
	// keep 10% of gravity as a kind of background burn
	DoWork(glm::length(f.acc - (0.9 * situation.GetPlanet().GravityAt(state.pos))) * Mass() * glm::length(f.vel) * dt);
}

Situation::Derivative Creature::Step(const Situation::Derivative &ds, double dt) const noexcept {
	Situation::State s = situation.GetState();
	s.pos += ds.vel * dt;
	s.vel += ds.acc * dt;
	situation.EnforceConstraints(s);
	glm::dvec3 force(steering.Force(s));
	// gravity = antinormal * mass * Gm / r²
	glm::dvec3 normal(situation.GetPlanet().NormalAt(s.pos));
	force += glm::dvec3(
		-normal
		* (Mass() * situation.GetPlanet().GravitationalParameter()
		/ glm::length2(s.pos)));
	// if net force is applied and in contact with surface
	if (!allzero(force) && !allzero(s.vel) && glm::length2(s.pos) < (situation.GetPlanet().Radius() + 0.01) * (situation.GetPlanet().Radius() + 0.01)) {
		// apply friction
		glm::dvec3 fn(normal * glm::dot(force, normal));
		// TODO: friction somehow bigger than force?
		glm::dvec3 ft(force - fn);
		double u = 0.4;
		glm::dvec3 friction(-glm::clamp(glm::length(ft), 0.0, glm::length(fn) * u) * glm::normalize(s.vel));
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
	Goal *top = &*goals.front();
	// if active goal can be interrupted, check priorities
	if (goals.size() > 1 && goals[0]->Interruptible()) {
		std::sort(goals.begin(), goals.end(), GoalCompare);
	}
	if (&*goals.front() != top) {
		top->SetBackground();
		goals.front()->SetForeground();
		top = &*goals.front();
	}
	goals[0]->Action();
	for (auto goal = goals.begin(); goal != goals.end();) {
		if ((*goal)->Complete()) {
			goals.erase(goal);
		} else {
			++goal;
		}
	}
	if (&*goals.front() != top) {
		goals.front()->SetForeground();
	}
}

math::AABB Creature::CollisionBounds() const noexcept {
	return { glm::dvec3(size * -0.5), glm::dvec3(size * 0.5) };
}

glm::dmat4 Creature::CollisionTransform() const noexcept {
	const double half_size = size * 0.5;
	const glm::dvec3 &pos = situation.Position();
	glm::dmat3 orient;
	orient[1] = situation.GetPlanet().NormalAt(pos);
	orient[2] = situation.Heading();
	if (std::abs(glm::dot(orient[1], orient[2])) > 0.999) {
		orient[2] = glm::dvec3(orient[1].z, orient[1].x, orient[1].y);
	}
	orient[0] = glm::normalize(glm::cross(orient[1], orient[2]));
	orient[2] = glm::normalize(glm::cross(orient[0], orient[1]));
	return glm::translate(glm::dvec3(pos.x, pos.y, pos.z))
		* glm::dmat4(orient)
		* glm::translate(glm::dvec3(0.0, half_size, 0.0));
}

void Creature::OnCollide(Creature &other) {
	memory.TrackCollision(other);
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
		constexpr float offset = 1.0f;
		constexpr float max_tex = 5.999f;
		const float tex[6] = {
			0.0f, // face
			float(std::floor(skin_side * max_tex)), // left
			float(std::floor(skin_back * max_tex)), // top
			float(std::floor(skin_back * max_tex)), // back
			float(std::floor(skin_side * max_tex)), // right
			0.0f, // bottom
		};
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
			attrib[4 * surface + 0].texture.z = tex[surface];

			attrib[4 * surface + 1].position[(surface + 0) % 3] = -offset;
			attrib[4 * surface + 1].position[(surface + 1) % 3] =  offset;
			attrib[4 * surface + 1].position[(surface + 2) % 3] = surface < 3 ? offset : -offset;
			attrib[4 * surface + 1].normal[(surface + 0) % 3] = 0.0f;
			attrib[4 * surface + 1].normal[(surface + 1) % 3] = 0.0f;
			attrib[4 * surface + 1].normal[(surface + 2) % 3] = surface < 3 ? 1.0f : -1.0f;
			attrib[4 * surface + 1].texture.x = tex_u_end;
			attrib[4 * surface + 1].texture.y = 1.0f;
			attrib[4 * surface + 1].texture.z = tex[surface];

			attrib[4 * surface + 2].position[(surface + 0) % 3] =  offset;
			attrib[4 * surface + 2].position[(surface + 1) % 3] = -offset;
			attrib[4 * surface + 2].position[(surface + 2) % 3] = surface < 3 ? offset : -offset;
			attrib[4 * surface + 2].normal[(surface + 0) % 3] = 0.0f;
			attrib[4 * surface + 2].normal[(surface + 1) % 3] = 0.0f;
			attrib[4 * surface + 2].normal[(surface + 2) % 3] = surface < 3 ? 1.0f : -1.0f;
			attrib[4 * surface + 2].texture.x = tex_u_begin;
			attrib[4 * surface + 2].texture.y = 0.0f;
			attrib[4 * surface + 2].texture.z = tex[surface];

			attrib[4 * surface + 3].position[(surface + 0) % 3] = offset;
			attrib[4 * surface + 3].position[(surface + 1) % 3] = offset;
			attrib[4 * surface + 3].position[(surface + 2) % 3] = surface < 3 ? offset : -offset;
			attrib[4 * surface + 3].normal[(surface + 0) % 3] = 0.0f;
			attrib[4 * surface + 3].normal[(surface + 1) % 3] = 0.0f;
			attrib[4 * surface + 3].normal[(surface + 2) % 3] = surface < 3 ? 1.0f : -1.0f;
			attrib[4 * surface + 3].texture.x = tex_u_end;
			attrib[4 * surface + 3].texture.y = 0.0f;
			attrib[4 * surface + 3].texture.z = tex[surface];
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
	c.GetSituation().SetPlanetSurface(p, glm::dvec3(0.0, 0.0, p.Radius()));
	c.GetSituation().Heading(glm::dvec3(1.0, 0.0, 0.0));
	c.HeadingTarget(glm::dvec3(1.0, 0.0, 0.0));

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

	genome.skin_side = { 0.5, 0.01 };
	genome.skin_back = { 0.5, 0.01 };

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
	c.BackSkin(glm::clamp(skin_back.FakeNormal(random.SNorm()), 0.0, 1.0));
	c.SideSkin(glm::clamp(skin_side.FakeNormal(random.SNorm()), 0.0, 1.0));
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
		s.GetPlanet(),
		s.Position() + glm::rotate(s.Heading() * a->Size() * 0.86, PI * 0.5, s.SurfaceNormal()));
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
		s.GetPlanet(),
		s.Position() + glm::rotate(s.Heading() * b->Size() * 0.86, PI * -0.5, s.SurfaceNormal()));
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
	known_creatures.clear();
}

bool Memory::RememberLocation(const Composition &accept, glm::dvec3 &pos) const noexcept {
	double best_rating = -1.0;
	for (const auto &k : known_types) {
		const world::TileType &t = c.GetSimulation().TileTypes()[k.first];
		auto entry = t.FindBestResource(accept);
		if (entry != t.resources.end()) {
			double rating = entry->ubiquity / std::max(0.125, 0.25 * glm::length2(c.GetSituation().Position() - k.second.first_loc.position));
			if (rating > best_rating) {
				best_rating = rating;
				pos = k.second.first_loc.position;
			}
			rating = entry->ubiquity / std::max(0.125, 0.25 * glm::length2(c.GetSituation().Position() - k.second.last_loc.position));
			if (rating > best_rating) {
				best_rating = rating;
				pos = k.second.last_loc.position;
			}
		}
	}
	if (best_rating > 0.0) {
		glm::dvec3 error(
			c.GetSimulation().Assets().random.SNorm(),
			c.GetSimulation().Assets().random.SNorm(),
			c.GetSimulation().Assets().random.SNorm());
		pos += error * (4.0 * (1.0 - c.IntelligenceFactor()));
		pos = glm::normalize(pos) * c.GetSituation().GetPlanet().Radius();
		return true;
	} else {
		return false;
	}
}

void Memory::TrackCollision(Creature &other) {
	// TODO: find out whose fault it was
	// TODO: source values from personality
	Profile &p = known_creatures[&other];
	p.annoyance += 0.1;
	const double annoy_fact = p.annoyance / (p.annoyance + 1.0);
	if (c.GetSimulation().Assets().random.UNorm() > annoy_fact * 0.1 * (1.0 - c.GetStats().Damage().value)) {
		AttackGoal *g = new AttackGoal(c, other);
		g->SetDamageTarget(annoy_fact);
		g->Urgency(annoy_fact);
		c.AddGoal(std::unique_ptr<Goal>(g));
		p.annoyance *= 0.5;
	}
}

void Memory::Tick(double dt) {
	Situation &s = c.GetSituation();
	if (s.OnSurface()) {
		TrackStay({ &s.GetPlanet(), s.Position() }, dt);
	}
	// TODO: forget
}

void Memory::TrackStay(const Location &l, double t) {
	const world::TileType &type = l.planet->TileTypeAt(l.position);
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

bool Situation::OnGround() const noexcept {
	return OnSurface() && glm::length2(state.pos) < (planet->Radius() + 0.05) * (planet->Radius() + 0.05);
}

glm::dvec3 Situation::SurfaceNormal() const noexcept {
	return planet->NormalAt(state.pos);
}

world::Tile &Situation::GetTile() const noexcept {
	return planet->TileAt(state.pos);
}

const world::TileType &Situation::GetTileType() const noexcept {
	return planet->TileTypeAt(state.pos);
}

void Situation::Move(const glm::dvec3 &dp) noexcept {
	state.pos += dp;
	EnforceConstraints(state);
}

void Situation::Accelerate(const glm::dvec3 &dv) noexcept {
	state.vel += dv;
	EnforceConstraints(state);
}

void Situation::EnforceConstraints(State &s) const noexcept {
	if (OnSurface()) {
		double r = GetPlanet().Radius();
		if (glm::length2(s.pos) < r * r) {
			const glm::dvec3 normal(GetPlanet().NormalAt(s.pos));
			s.pos = normal * r;
			s.vel -= normal * glm::dot(normal, s.vel);
		}
	}
}

void Situation::SetPlanetSurface(world::Planet &p, const glm::dvec3 &pos) noexcept {
	type = PLANET_SURFACE;
	planet = &p;
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
			if (glm::length2(diff) > max_look * max_look) continue;
			if (!c.PerceptionTest(other->GetSituation().Position())) continue;
			double sep = glm::clamp(glm::length(diff) - other->Size() * 0.707 - c.Size() * 0.707, 0.0, min_dist);
			repulse += glm::normalize(diff) * (1.0 - sep / min_dist) * force;
		}
		result += repulse;
	}
	if (halting) {
		// brake hard
		result += -5.0 * s.vel * force;
	}
	if (seeking) {
		glm::dvec3 diff = target - s.pos;
		if (!allzero(diff)) {
			result += TargetVelocity(s, (glm::normalize(diff) * speed), force);
		}
	}
	if (arriving) {
		glm::dvec3 diff = target - s.pos;
		double dist = glm::length(diff);
		if (!allzero(diff) && dist > std::numeric_limits<double>::epsilon()) {
			result += TargetVelocity(s, diff * std::min(dist * force, speed) / dist, force);
		}
	}
	// remove vertical component, if any
	const glm::dvec3 normal(c.GetSituation().GetPlanet().NormalAt(s.pos));
	result -= normal * glm::dot(normal, result);
	// clamp to max
	if (glm::length2(result) > max_force * max_force) {
		result = glm::normalize(result) * max_force;
	}
	return result;
}

glm::dvec3 Steering::TargetVelocity(const Situation::State &s, const glm::dvec3 &vel, double acc) const noexcept {
	return (vel - s.vel) * acc;
}

}
}
