#include "AttackGoal.hpp"
#include "BlobBackgroundTask.hpp"
#include "Goal.hpp"
#include "IdleGoal.hpp"
#include "IngestGoal.hpp"
#include "LocateResourceGoal.hpp"
#include "LookAroundGoal.hpp"
#include "StrollGoal.hpp"

#include "Creature.hpp"
#include "../app/Assets.hpp"
#include "../math/const.hpp"
#include "../ui/string.hpp"
#include "../world/Planet.hpp"
#include "../world/Resource.hpp"
#include "../world/Simulation.hpp"
#include "../world/TileType.hpp"

#include <cstring>
#include <iostream>
#include <sstream>
#include <glm/gtx/io.hpp>
#include <glm/gtx/rotate_vector.hpp>


namespace blobs {
namespace creature {

AttackGoal::AttackGoal(Creature &self, Creature &target)
: Goal(self)
, target(target)
, damage_target(0.25)
, damage_dealt(0.0)
, cooldown(0.0) {
}

AttackGoal::~AttackGoal() {
}

std::string AttackGoal::Describe() const {
	return "attack " + target.Name();
}

void AttackGoal::Tick(double dt) {
	cooldown -= dt;
}

void AttackGoal::Action() {
	if (target.Dead() || !GetCreature().PerceptionTest(target.GetSituation().Position())) {
		SetComplete();
		return;
	}
	const glm::dvec3 diff(GetSituation().Position() - target.GetSituation().Position());
	const double hit_range = GetCreature().Size() * 0.5 * GetCreature().DexertyFactor();
	const double hit_dist = hit_range + (0.5 * GetCreature().Size()) + 0.5 * (target.Size());
	if (GetStats().Damage().Critical()) {
		// flee
		GetSteering().Pass(diff * 5.0);
		GetSteering().DontSeparate();
		GetSteering().Haste(1.0);
	} else if (glm::length2(diff) > hit_dist * hit_dist) {
		// full throttle chase
		GetSteering().Pass(target.GetSituation().Position());
		GetSteering().DontSeparate();
		GetSteering().Haste(1.0);
	} else {
		// attack
		GetSteering().Halt();
		GetSteering().DontSeparate();
		GetSteering().Haste(1.0);
		if (cooldown <= 0.0) {
			constexpr double impulse = 0.05;
			const double force = GetCreature().Strength();
			const double damage =
				force * impulse
				* (GetCreature().GetComposition().TotalDensity() / target.GetComposition().TotalDensity())
				* (GetCreature().Mass() / target.Mass())
				/ target.Mass();
			GetCreature().DoWork(force * impulse * glm::length(diff));
			target.Hurt(damage);
			target.GetSituation().Accelerate(glm::normalize(diff) * force * -impulse);
			damage_dealt += damage;
			if (damage_dealt >= damage_target || target.Dead()) {
				SetComplete();
				if (target.Dead()) {
					GetCreature().GetSimulation().Log() << GetCreature().Name()
						<< " killed " << target.Name() << std::endl;
				}
			}
			cooldown = 1.0 + (4.0 * (1.0 - GetCreature().DexertyFactor()));
		}
	}
}

void AttackGoal::OnBackground() {
	// abort if something more important comes up
	SetComplete();
}


BlobBackgroundTask::BlobBackgroundTask(Creature &c)
: Goal(c)
, breathing(false)
, drink_subtask(nullptr)
, eat_subtask(nullptr) {
}

BlobBackgroundTask::~BlobBackgroundTask() {
}

std::string BlobBackgroundTask::Describe() const {
	return "being a blob";
}

void BlobBackgroundTask::Tick(double dt) {
	if (breathing) {
		// TODO: derive breathing ability
		int gas = Assets().data.resources["air"].id;
		// TODO: check if in compatible atmosphere
		double amount = GetStats().Breath().gain * -(1.0 + GetCreature().ExhaustionFactor());
		GetStats().Breath().Add(amount * dt);
		// maintain ~1% gas composition
		double gas_amount = GetCreature().GetComposition().Get(gas);
		if (gas_amount < GetCreature().GetComposition().TotalMass() * 0.01) {
			double add = std::min(GetCreature().GetComposition().TotalMass() * 0.025 - gas_amount, -amount * dt);
			GetCreature().Ingest(gas, add);
		}
		if (GetStats().Breath().Empty()) {
			breathing = false;
		}
	}
}

void BlobBackgroundTask::Action() {
	CheckStats();
	CheckSplit();
	CheckMutate();
}

void BlobBackgroundTask::CheckStats() {
	Creature::Stats &stats = GetStats();

	if (!breathing && stats.Breath().Bad()) {
		breathing = true;
	}

	if (!drink_subtask && stats.Thirst().Bad()) {
		drink_subtask = new IngestGoal(GetCreature(), stats.Thirst());
		for (const auto &cmp : GetCreature().GetComposition()) {
			if (Assets().data.resources[cmp.resource].state == world::Resource::LIQUID) {
				double value = cmp.value / GetCreature().GetComposition().TotalMass();
				drink_subtask->Accept(cmp.resource, value);
				for (const auto &compat : Assets().data.resources[cmp.resource].compatibility) {
					if (Assets().data.resources[compat.first].state == world::Resource::LIQUID) {
						drink_subtask->Accept(compat.first, value * compat.second);
					}
				}
			}
		}
		drink_subtask->WhenComplete([&](Goal &) { drink_subtask = nullptr; });
		GetCreature().AddGoal(std::unique_ptr<Goal>(drink_subtask));
	}

	if (!eat_subtask && stats.Hunger().Bad()) {
		eat_subtask = new IngestGoal(GetCreature(), stats.Hunger());
		for (const auto &cmp : GetCreature().GetComposition()) {
			if (Assets().data.resources[cmp.resource].state == world::Resource::SOLID) {
				double value = cmp.value / GetCreature().GetComposition().TotalMass();
				eat_subtask->Accept(cmp.resource, value);
				for (const auto &compat : Assets().data.resources[cmp.resource].compatibility) {
					if (Assets().data.resources[compat.first].state == world::Resource::SOLID) {
						eat_subtask->Accept(compat.first, value * compat.second);
					}
				}
			}
		}
		eat_subtask->WhenComplete([&](Goal &) { eat_subtask = nullptr; });
		GetCreature().AddGoal(std::unique_ptr<Goal>(eat_subtask));
	}
}

void BlobBackgroundTask::CheckSplit() {
	if (GetCreature().Mass() > GetCreature().OffspringMass() * 2.0
		&& GetCreature().OffspringChance() > Random().UNorm()) {
		GetCreature().GetSimulation().Log() << GetCreature().Name() << " split" << std::endl;
		Split(GetCreature());
		return;
	}
}

void BlobBackgroundTask::CheckMutate() {
	// check for random property mutation
	if (GetCreature().MutateChance() > Random().UNorm()) {
		double amount = 1.0 + (Random().SNorm() * 0.05);
		math::Distribution &d = GetCreature().GetGenome().properties.props[Random().UInt(9)];
		if (Random().UNorm() < 0.5) {
			d.Mean(d.Mean() * amount);
		} else {
			d.StandardDeviation(d.StandardDeviation() * amount);
		}
	}
}


Goal::Goal(Creature &c)
: c(c)
, on_complete()
, on_foreground()
, on_background()
, urgency(0.0)
, interruptible(true)
, complete(false) {
}

Goal::~Goal() noexcept {
}

app::Assets &Goal::Assets() noexcept {
	return c.GetSimulation().Assets();
}

const app::Assets &Goal::Assets() const noexcept {
	return c.GetSimulation().Assets();
}

math::GaloisLFSR &Goal::Random() noexcept {
	return Assets().random;
}

void Goal::SetComplete() {
	if (!complete) {
		complete = true;
		OnComplete();
		if (on_complete) {
			on_complete(*this);
		}
	}
}

void Goal::SetForeground() {
	OnForeground();
	if (on_foreground) {
		on_foreground(*this);
	}
}

void Goal::SetBackground() {
	OnBackground();
	if (on_background) {
		on_background(*this);
	}
}

void Goal::WhenComplete(std::function<void(Goal &)> cb) noexcept {
	on_complete = cb;
	if (complete) {
		on_complete(*this);
	}
}

void Goal::WhenForeground(std::function<void(Goal &)> cb) noexcept {
	on_foreground = cb;
}

void Goal::WhenBackground(std::function<void(Goal &)> cb) noexcept {
	on_background = cb;
}


IdleGoal::IdleGoal(Creature &c)
: Goal(c) {
	Urgency(-1.0);
	Interruptible(true);
}

IdleGoal::~IdleGoal() {
}

std::string IdleGoal::Describe() const {
	return "idle";
}

void IdleGoal::Action() {
	// when in bad shape, don't make much effort
	if (GetStats().Damage().Bad() || GetStats().Exhaustion().Bad() || GetStats().Fatigue().Critical()) {
		GetSteering().DontSeparate();
	} else {
		GetSteering().ResumeSeparate();
	}

	// use boredom as chance per 15s
	if (Random().UNorm() < GetStats().Boredom().value * (1.0 / 900.0)) {
		PickActivity();
	}
}

void IdleGoal::PickActivity() {
	int n = Random().UInt(2);
	if (n == 0) {
		GetCreature().AddGoal(std::unique_ptr<Goal>(new StrollGoal(GetCreature())));
	} else {
		GetCreature().AddGoal(std::unique_ptr<Goal>(new LookAroundGoal(GetCreature())));
	}
}


namespace {

std::string summarize(const Composition &comp, const app::Assets &assets) {
	std::stringstream s;
	bool first = true;
	for (const auto &c : comp) {
		if (first) {
			first = false;
		} else {
			s << " or ";
		}
		s << assets.data.resources[c.resource].label;
	}
	return s.str();
}

}

IngestGoal::IngestGoal(Creature &c, Creature::Stat &stat)
: Goal(c)
, stat(stat)
, accept(Assets().data.resources)
, locate_subtask(nullptr)
, ingesting(false)
, resource(-1)
, yield(0.0) {
	Urgency(stat.value);
}

IngestGoal::~IngestGoal() {
}

void IngestGoal::Accept(int resource, double value) {
	accept.Add(resource, value);
}

std::string IngestGoal::Describe() const {
	if (resource == -1) {
		return "ingest " + summarize(accept, Assets());
	} else {
		const world::Resource &r = Assets().data.resources[resource];
		if (r.state == world::Resource::SOLID) {
			return "eat " + r.label;
		} else {
			return "drink " + r.label;
		}
	}
}

void IngestGoal::Enable() {
}

void IngestGoal::Tick(double dt) {
	Urgency(stat.value);
	if (locate_subtask) {
		locate_subtask->Urgency(Urgency() + 0.1);
	}
	if (ingesting) {
		if (OnSuitableTile() && !GetSituation().Moving()) {
			GetCreature().Ingest(resource, yield * dt);
			stat.Add(-1.0 * yield * GetCreature().GetComposition().Compatibility(resource) * dt);
			if (stat.Empty()) {
				SetComplete();
			}
		} else {
			// left tile somehow, some idiot probably pushed us off
			ingesting = false;
			Interruptible(true);
		}
	}
}

void IngestGoal::Action() {
	if (ingesting) {
		// all good
		return;
	}
	if (OnSuitableTile()) {
		if (GetSituation().Moving()) {
			// break with maximum force
			GetSteering().Haste(1.0);
			GetSteering().Halt();
		} else {
			// finally
			// TODO: somehow this still gets interrupted
			Interruptible(false);
			ingesting = true;
		}
	} else {
		locate_subtask = new LocateResourceGoal(GetCreature());
		for (const auto &c : accept) {
			locate_subtask->Accept(c.resource, c.value);
		}
		locate_subtask->SetMinimum(stat.gain * -1.1);
		locate_subtask->Urgency(Urgency() + 0.1);
		locate_subtask->WhenComplete([&](Goal &){ locate_subtask = nullptr; });
		GetCreature().AddGoal(std::unique_ptr<Goal>(locate_subtask));
	}
}

bool IngestGoal::OnSuitableTile() {
	if (!GetSituation().OnGround()) {
		return false;
	}
	const world::TileType &t = GetSituation().GetTileType();
	auto found = t.FindBestResource(accept);
	if (found != t.resources.end()) {
		resource = found->resource;
		yield = found->ubiquity;
		return true;
	} else {
		resource = -1;
		return false;
	}
}


LocateResourceGoal::LocateResourceGoal(Creature &c)
: Goal(c)
, accept(Assets().data.resources)
, found(false)
, target_pos(0.0)
, searching(false)
, reevaluate(0.0)
, minimum(0.0) {
}

LocateResourceGoal::~LocateResourceGoal() noexcept {
}

void LocateResourceGoal::Accept(int resource, double value) {
	accept.Add(resource, value);
}

std::string LocateResourceGoal::Describe() const {
	return "locate " + summarize(accept, Assets());
}

void LocateResourceGoal::Enable() {

}

void LocateResourceGoal::Tick(double dt) {
	reevaluate -= dt;
}

void LocateResourceGoal::Action() {
	if (reevaluate < 0.0) {
		LocateResource();
		reevaluate = 3.0;
	} else if (!found) {
		if (!searching) {
			LocateResource();
		} else {
			if (OnTarget()) {
				searching = false;
				LocateResource();
			} else {
				GetSteering().GoTo(target_pos);
			}
		}
	} else if (OnTarget()) {
		GetSteering().Halt();
		if (!GetSituation().Moving()) {
			SetComplete();
		}
	} else {
		GetSteering().GoTo(target_pos);
	}
	GetSteering().Haste(Urgency());
}

void LocateResourceGoal::LocateResource() {
	if (GetSituation().OnSurface()) {
		const world::TileType &t = GetSituation().GetTileType();
		auto yield = t.FindBestResource(accept);
		if (yield != t.resources.cend()) {
			// hoooray
			GetSteering().Halt();
			found = true;
			searching = false;
			target_pos = GetSituation().Position();
		} else {
			// go find somewhere else
			SearchVicinity();
			if (!found) {
				Remember();
				if (!found) {
					RandomWalk();
				}
			}
		}
	} else {
		// well, what now?
		found = false;
		searching = false;
	}
}

void LocateResourceGoal::SearchVicinity() {
	const world::Planet &planet = GetSituation().GetPlanet();
	const glm::dvec3 &pos = GetSituation().Position();
	const glm::dvec3 normal(planet.NormalAt(pos));
	const glm::dvec3 step_x(glm::normalize(glm::cross(normal, glm::dvec3(normal.z, normal.x, normal.y))) * (GetCreature().PerceptionOmniRange() * 0.7));
	const glm::dvec3 step_y(glm::normalize(glm::cross(step_x, normal)) * (GetCreature().PerceptionOmniRange() * 0.7));

	const int search_radius = int(GetCreature().PerceptionRange() / (GetCreature().PerceptionOmniRange() * 0.7));
	double rating[2 * search_radius + 1][2 * search_radius + 1];
	std::memset(rating, '\0', (2 * search_radius + 1) * (2 * search_radius + 1) * sizeof(double));

	// find close and rich field
	for (int y = -search_radius; y < search_radius + 1; ++y) {
		for (int x = -search_radius; x < search_radius + 1; ++x) {
			const glm::dvec3 tpos(pos + (double(x) * step_x) + (double(y) * step_y));
			if (!GetCreature().PerceptionTest(tpos)) continue;
			const world::TileType &type = planet.TileTypeAt(tpos);
			auto yield = type.FindBestResource(accept);
			if (yield != type.resources.cend()) {
				rating[y + search_radius][x + search_radius] = yield->ubiquity * accept.Get(yield->resource);
				// penalize distance
				double dist = std::max(0.125, 0.25 * glm::length2(tpos - pos));
				rating[y + search_radius][x + search_radius] /= dist;
			}
		}
	}

	// penalize crowding
	for (auto &c : planet.Creatures()) {
		if (&*c == &GetCreature()) continue;
		for (int y = -search_radius; y < search_radius + 1; ++y) {
			for (int x = -search_radius; x < search_radius + 1; ++x) {
				const glm::dvec3 tpos(pos + (double(x) * step_x) + (double(y) * step_y));
				if (glm::length2(tpos - c->GetSituation().Position()) < 1.0) {
					rating[y + search_radius][x + search_radius] *= 0.8;
				}
			}
		}
	}

	glm::ivec2 best_pos(0);
	double best_rating = -1.0;

	for (int y = -search_radius; y < search_radius + 1; ++y) {
		for (int x = -search_radius; x < search_radius + 1; ++x) {
			if (rating[y + search_radius][x + search_radius] > best_rating) {
				best_pos = glm::ivec2(x, y);
				best_rating = rating[y + search_radius][x + search_radius];
			}
		}
	}

	if (best_rating > minimum) {
		found = true;
		searching = false;
		target_pos = glm::normalize(pos + (double(best_pos.x) * step_x) + (double(best_pos.y) * step_y)) * planet.Radius();
		GetSteering().GoTo(target_pos);
	}
}

void LocateResourceGoal::Remember() {
	glm::dvec3 pos(0.0);
	if (GetCreature().GetMemory().RememberLocation(accept, pos)) {
		found = true;
		searching = false;
		target_pos = pos;
		GetSteering().GoTo(target_pos);
	}
}

void LocateResourceGoal::RandomWalk() {
	if (searching) {
		return;
	}

	const world::Planet &planet = GetSituation().GetPlanet();
	const glm::dvec3 &pos = GetSituation().Position();
	const glm::dvec3 normal(planet.NormalAt(pos));
	const glm::dvec3 step_x(glm::normalize(glm::cross(normal, glm::dvec3(normal.z, normal.x, normal.y))));
	const glm::dvec3 step_y(glm::normalize(glm::cross(step_x, normal)));

	found = false;
	searching = true;
	target_pos = GetSituation().Position();
	target_pos += Random().SNorm() * 3.0 * step_x;
	target_pos += Random().SNorm() * 3.0 * step_y;
	// bias towards current heading
	target_pos += GetSituation().Heading() * 1.5;
	target_pos = glm::normalize(target_pos) * planet.Radius();
	GetSteering().GoTo(target_pos);
}

bool LocateResourceGoal::OnTarget() const noexcept {
	const Situation &s = GetSituation();
	return s.OnGround() && glm::length2(s.Position() - target_pos) < 0.0001;
}


LookAroundGoal::LookAroundGoal(Creature &c)
: Goal(c)
, timer(0.0) {
}

LookAroundGoal::~LookAroundGoal() {
}

std::string LookAroundGoal::Describe() const {
	return "look around";
}

void LookAroundGoal::Enable() {
	GetSteering().Halt();
}

void LookAroundGoal::Tick(double dt) {
	timer -= dt;
}

void LookAroundGoal::Action() {
	if (timer < 0.0) {
		PickDirection();
		timer = 1.0 + (Random().UNorm() * 4.0);
	}
}

void LookAroundGoal::OnBackground() {
	SetComplete();
}

void LookAroundGoal::PickDirection() noexcept {
	double r = Random().SNorm();
	r *= std::abs(r) * 0.5 * PI;
	GetCreature().HeadingTarget(glm::rotate(GetSituation().Heading(), r, GetSituation().SurfaceNormal()));
}


StrollGoal::StrollGoal(Creature &c)
: Goal(c)
, last(GetSituation().Position())
, next(last) {
}

StrollGoal::~StrollGoal() {
}

std::string StrollGoal::Describe() const {
	return "take a walk";
}

void StrollGoal::Enable() {
	last = GetSituation().Position();
	GetSteering().Haste(0.0);
	PickTarget();
}

void StrollGoal::Action() {
	if (glm::length2(next - GetSituation().Position()) < 0.0001) {
		PickTarget();
	}
}

void StrollGoal::OnBackground() {
	SetComplete();
}

void StrollGoal::PickTarget() noexcept {
	last = next;
	next += GetSituation().Heading() * 1.5;
	const glm::dvec3 normal(GetSituation().GetPlanet().NormalAt(GetSituation().Position()));
	glm::dvec3 rand_x(GetSituation().Heading());
	if (std::abs(glm::dot(normal, rand_x)) > 0.999) {
		rand_x = glm::dvec3(normal.z, normal.x, normal.y);
	}
	glm::dvec3 rand_y = glm::cross(normal, rand_x);
	next += ((rand_x * Random().SNorm()) + (rand_y * Random().SNorm())) * 1.5;
	next = glm::normalize(next) * GetSituation().GetPlanet().Radius();
	GetSteering().GoTo(next);
}

}
}
