#include "BlobBackgroundTask.hpp"
#include "Goal.hpp"
#include "IdleGoal.hpp"
#include "IngestGoal.hpp"
#include "LocateResourceGoal.hpp"
#include "StrollGoal.hpp"

#include "Creature.hpp"
#include "../app/Assets.hpp"
#include "../ui/string.hpp"
#include "../world/Planet.hpp"
#include "../world/Resource.hpp"
#include "../world/Simulation.hpp"
#include "../world/TileType.hpp"

#include <cstring>
#include <iostream>
#include <sstream>
#include <glm/gtx/io.hpp>


namespace blobs {
namespace creature {

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
		double amount = GetCreature().GetStats().Breath().gain * -(1.0 + GetCreature().ExhaustionFactor());
		GetCreature().GetStats().Breath().Add(amount * dt);
		// maintain ~1% gas composition
		double gas_amount = GetCreature().GetComposition().Get(gas);
		if (gas_amount < GetCreature().GetComposition().TotalMass() * 0.01) {
			double add = std::min(GetCreature().GetComposition().TotalMass() * 0.025 - gas_amount, -amount * dt);
			GetCreature().Ingest(gas, add);
		}
		if (GetCreature().GetStats().Breath().Empty()) {
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
	Creature::Stats &stats = GetCreature().GetStats();

	if (!breathing && stats.Breath().Bad()) {
		breathing = true;
	}

	if (!drink_subtask && stats.Thirst().Bad()) {
		drink_subtask = new IngestGoal(GetCreature(), stats.Thirst());
		for (const auto &cmp : GetCreature().GetComposition()) {
			if (Assets().data.resources[cmp.resource].state == world::Resource::LIQUID) {
				drink_subtask->Accept(cmp.resource, 1.0);
			}
		}
		drink_subtask->WhenComplete([&](Goal &) { drink_subtask = nullptr; });
		GetCreature().AddGoal(std::unique_ptr<Goal>(drink_subtask));
	}

	if (!eat_subtask && stats.Hunger().Bad()) {
		eat_subtask = new IngestGoal(GetCreature(), stats.Hunger());
		for (const auto &cmp : GetCreature().GetComposition()) {
			if (Assets().data.resources[cmp.resource].state == world::Resource::SOLID) {
				eat_subtask->Accept(cmp.resource, 1.0);
			}
		}
		eat_subtask->WhenComplete([&](Goal &) { eat_subtask = nullptr; });
		GetCreature().AddGoal(std::unique_ptr<Goal>(eat_subtask));
	}

	// when in bad shape, don't make much effort
	if (stats.Damage().Bad() || stats.Exhaustion().Bad() || stats.Fatigue().Critical()) {
		GetCreature().GetSteering().DontSeparate();
	} else {
		GetCreature().GetSteering().ResumeSeparate();
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

Situation &Goal::GetSituation() noexcept {
	return c.GetSituation();
}

const Situation &Goal::GetSituation() const noexcept {
	return c.GetSituation();
}

Steering &Goal::GetSteering() noexcept {
	return c.GetSteering();
}

const Steering &Goal::GetSteering() const noexcept {
	return c.GetSteering();
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
	// use boredom as chance per minute
	if (Random().UNorm() < GetCreature().GetStats().Boredom().value * (1.0 / 3600.0)) {
		PickActivity();
	}
}

void IdleGoal::PickActivity() {
	GetCreature().AddGoal(std::unique_ptr<Goal>(new StrollGoal(GetCreature())));
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
, accept()
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
			// TODO: determine satisfaction factor
			GetCreature().Ingest(resource, yield * dt);
			stat.Add(-1.0 * yield * dt);
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
		locate_subtask->Urgency(Urgency() + 0.1);
		locate_subtask->WhenComplete([&](Goal &){ locate_subtask = nullptr; });
		GetCreature().AddGoal(std::unique_ptr<Goal>(locate_subtask));
	}
}

bool IngestGoal::OnSuitableTile() {
	if (!GetSituation().OnSurface()) {
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
, accept()
, found(false)
, target_pos(0.0)
, searching(false)
, reevaluate(0.0) {
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
			double dist = glm::length2(GetSituation().Position() - target_pos);
			if (dist < 0.0001) {
				searching = false;
				LocateResource();
			} else {
				GetSteering().GoTo(target_pos);
			}
		}
	} else if (NearTarget()) {
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
	const glm::dvec3 step_x(normalize(cross(normal, glm::dvec3(normal.z, normal.x, normal.y))));
	const glm::dvec3 step_y(normalize(cross(step_x, normal)));

	constexpr int search_radius = 2;
	double rating[2 * search_radius + 1][2 * search_radius + 1] = {0};

	// find close and rich field
	for (int y = -search_radius; y < search_radius + 1; ++y) {
		for (int x = -search_radius; x < search_radius + 1; ++x) {
			const glm::dvec3 tpos(pos + (double(x) * step_x) + (double(y) * step_y));
			if (!GetCreature().PerceptionTest(tpos)) continue;
			const world::TileType &type = planet.TileTypeAt(tpos);
			auto yield = type.FindBestResource(accept);
			if (yield != type.resources.cend()) {
				// TODO: subtract minimum yield
				rating[y + search_radius][x + search_radius] = yield->ubiquity * accept.Get(yield->resource);
				// penalize distance
				double dist = std::max(0.125, 0.25 * glm::length(tpos - pos));
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
				if (length2(tpos - c->GetSituation().Position()) < 1.0) {
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

	if (best_rating > 0.0) {
		found = true;
		searching = false;
		target_pos = normalize(pos + (double(best_pos.x) * step_x) + (double(best_pos.y) * step_y)) * planet.Radius();
		GetSteering().GoTo(target_pos);
	} else if (!searching) {
		found = false;
		searching = true;
		target_pos = GetSituation().Position();
		target_pos += Random().SNorm() * step_x;
		target_pos += Random().SNorm() * step_y;
		// bias towards current heading
		target_pos += GetSituation().Heading() * 1.5;
		target_pos = normalize(target_pos) * planet.Radius();
		GetSteering().GoTo(target_pos);
	}
}

bool LocateResourceGoal::NearTarget() const noexcept {
	const Situation &s = GetSituation();
	return s.OnSurface() && length2(s.Position() - target_pos) < 0.5;
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
	if (length2(next - GetSituation().Position()) < 0.0001) {
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
	if (std::abs(dot(normal, rand_x)) > 0.999) {
		rand_x = glm::dvec3(normal.z, normal.x, normal.y);
	}
	glm::dvec3 rand_y = cross(normal, rand_x);
	next += ((rand_x * Random().SNorm()) + (rand_y * Random().SNorm())) * 1.5;
	next = normalize(next) * GetSituation().GetPlanet().Radius();
	GetSteering().GoTo(next);
}

}
}
