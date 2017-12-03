#include "BlobBackgroundTask.hpp"
#include "Goal.hpp"
#include "IdleGoal.hpp"
#include "IngestGoal.hpp"
#include "LocateResourceGoal.hpp"

#include "Creature.hpp"
#include "../app/Assets.hpp"
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
		double amount = GetCreature().GetStats().Breath().gain * -(1.5 + 0.5 * GetCreature().ExhaustionFactor());
		GetCreature().GetStats().Breath().Add(amount * dt);
		// maintain ~2.5% gas composition
		double gas_amount = GetCreature().GetComposition().Get(gas);
		if (gas_amount < GetCreature().GetComposition().TotalMass() * 0.025) {
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
		drink_subtask->OnComplete([&](Goal &) { drink_subtask = nullptr; });
		GetCreature().AddGoal(std::unique_ptr<Goal>(drink_subtask));
	}

	if (!eat_subtask && stats.Hunger().Bad()) {
		eat_subtask = new IngestGoal(GetCreature(), stats.Hunger());
		for (const auto &cmp : GetCreature().GetComposition()) {
			if (Assets().data.resources[cmp.resource].state == world::Resource::SOLID) {
				eat_subtask->Accept(cmp.resource, 1.0);
			}
		}
		eat_subtask->OnComplete([&](Goal &) { eat_subtask = nullptr; });
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
		&& GetCreature().OffspringChance() > Assets().random.UNorm()) {
		std::cout << "[" << int(GetCreature().GetSimulation().Time())
			<< "s] " << GetCreature().Name() << " split" << std::endl;
		Split(GetCreature());
		return;
	}
}

void BlobBackgroundTask::CheckMutate() {
	// check for random property mutation
	if (GetCreature().MutateChance() > Assets().random.UNorm()) {
		double amount = 1.0 + (Assets().random.SNorm() * 0.05);
		math::Distribution &d = GetCreature().GetGenome().properties.props[(int(Assets().random.UNorm() * 8.0) % 8)];
		if (Assets().random.UNorm() < 0.5) {
			d.Mean(d.Mean() * amount);
		} else {
			d.StandardDeviation(d.StandardDeviation() * amount);
		}
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
			Interruptible(false);
			ingesting = true;
		}
	} else {
		locate_subtask = new LocateResourceGoal(GetCreature());
		for (const auto &c : accept) {
			locate_subtask->Accept(c.resource, c.value);
		}
		locate_subtask->Urgency(Urgency() + 0.1);
		locate_subtask->OnComplete([&](Goal &){ locate_subtask = nullptr; });
		GetCreature().AddGoal(std::unique_ptr<Goal>(locate_subtask));
	}
}

bool IngestGoal::OnSuitableTile() {
	if (!GetSituation().OnTile()) {
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


Goal::Goal(Creature &c)
: c(c)
, on_complete()
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

void Goal::SetComplete() noexcept {
	if (!complete) {
		complete = true;
		if (on_complete) {
			on_complete(*this);
		}
	}
}

void Goal::OnComplete(std::function<void(Goal &)> cb) noexcept {
	on_complete = cb;
	if (complete) {
		on_complete(*this);
	}
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

void IdleGoal::Enable() {
}

void IdleGoal::Tick(double dt) {
}

void IdleGoal::Action() {
}


LocateResourceGoal::LocateResourceGoal(Creature &c)
: Goal(c)
, accept()
, found(false)
, target_pos(0.0)
, target_srf(0)
, target_tile(0)
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
				LocateResource();
			} else {
				GetSteering().GoTo(target_pos);
			}
		}
	} else if (OnTargetTile()) {
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
	if (GetSituation().OnTile()) {
		const world::TileType &t = GetSituation().GetTileType();
		auto yield = t.FindBestResource(accept);
		if (yield != t.resources.cend()) {
			// hoooray
			GetSteering().Halt();
			found = true;
			searching = false;
			target_pos = GetSituation().Position();
			target_srf = GetSituation().Surface();
			target_tile = GetSituation().GetPlanet().SurfacePosition(target_srf, target_pos);
		} else {
			// go find somewhere else
			SearchVicinity();
		}
	} else {
		// well, what now?
	}
}

void LocateResourceGoal::SearchVicinity() {
	const world::Planet &planet = GetSituation().GetPlanet();
	int srf = GetSituation().Surface();
	const glm::dvec3 &pos = GetSituation().Position();

	glm::ivec2 loc = planet.SurfacePosition(srf, pos);
	glm::ivec2 seek_radius(2);
	glm::ivec2 begin(glm::max(glm::ivec2(0), loc - seek_radius));
	glm::ivec2 end(glm::min(glm::ivec2(planet.SideLength()), loc + seek_radius + glm::ivec2(1)));

	double rating[end.y - begin.y][end.x - begin.x];
	std::memset(rating, 0, sizeof(double) * (end.y - begin.y) * (end.x - begin.x));

	// find close and rich field
	for (int y = begin.y; y < end.y; ++y) {
		for (int x = begin.x; x < end.x; ++x) {
			const world::TileType &type = planet.TypeAt(srf, x, y);
			auto yield = type.FindBestResource(accept);
			if (yield != type.resources.cend()) {
				// TODO: subtract minimum yield
				rating[y - begin.y][x - begin.x] = yield->ubiquity * accept.Get(yield->resource);
				double dist = std::max(0.125, 0.25 * glm::length(planet.TileCenter(srf, x, y) - pos));
				rating[y - begin.y][x - begin.x] /= dist;
			}
		}
	}

	// demote crowded tiles
	for (auto &c : planet.Creatures()) {
		if (&*c == &GetCreature()) continue;
		if (c->GetSituation().Surface() != srf) continue;
		glm::ivec2 coords(c->GetSituation().SurfacePosition());
		if (coords.x < begin.x || coords.x >= end.x) continue;
		if (coords.y < begin.y || coords.y >= end.y) continue;
		rating[coords.y - begin.y][coords.x - begin.x] *= 0.9;
	}

	glm::ivec2 best_pos(0);
	double best_rating = -1.0;

	for (int y = begin.y; y < end.y; ++y) {
		for (int x = begin.x; x < end.x; ++x) {
			if (rating[y - begin.y][x - begin.x] > best_rating) {
				best_pos = glm::ivec2(x, y);
				best_rating = rating[y - begin.y][x - begin.x];
			}
		}
	}

	if (best_rating) {
		found = true;
		searching = false;
		target_pos = planet.TileCenter(srf, best_pos.x, best_pos.y);
		target_srf = srf;
		target_tile = best_pos;
		GetSteering().GoTo(target_pos);
	} else if (!searching) {
		found = false;
		searching = true;
		target_pos = GetSituation().Position();
		target_pos[(srf + 0) % 3] += Assets().random.SNorm();
		target_pos[(srf + 1) % 3] += Assets().random.SNorm();
		// bias towards current direction
		target_pos += glm::normalize(GetSituation().Velocity()) * 0.5;
		target_pos = clamp(target_pos, -planet.Radius(), planet.Radius());
		GetSteering().GoTo(target_pos);
	}
}

bool LocateResourceGoal::OnTargetTile() const noexcept {
	const Situation &s = GetSituation();
	return s.OnSurface()
		&& s.Surface() == target_srf
		&& s.OnTile()
		&& s.SurfacePosition() == target_tile;
}

}
}
