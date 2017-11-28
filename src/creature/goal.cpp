#include "Goal.hpp"
#include "IdleGoal.hpp"
#include "LocateResourceGoal.hpp"

#include "Creature.hpp"
#include "../app/Assets.hpp"
#include "../world/Planet.hpp"
#include "../world/Resource.hpp"
#include "../world/Simulation.hpp"
#include "../world/TileType.hpp"

#include <iostream>
#include <glm/gtx/io.hpp>


namespace blobs {
namespace creature {

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
	double fert = GetCreature().Fertility();
	double rand = Assets().random.UNorm();
	if (fert > rand) {
		std::cout << "[" << int(GetCreature().GetSimulation().Time())
			<< "s] splitting " << GetCreature().Name()
			<< " because " << fert << " > " << rand << std::endl;
		Split(GetCreature());
	}
}


LocateResourceGoal::LocateResourceGoal(Creature &c, int res)
: Goal(c)
, res(res)
, found(false)
, target_pos(0.0)
, target_srf(0)
, target_tile(0)
, searching(false) {
}

LocateResourceGoal::~LocateResourceGoal() noexcept {
}

std::string LocateResourceGoal::Describe() const {
	return "locate " + GetCreature().GetSimulation().Resources()[res].name;
}

void LocateResourceGoal::Enable() {
	LocateResource();
}

void LocateResourceGoal::Tick(double dt) {
}

void LocateResourceGoal::Action() {
	if (!found) {
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
}

void LocateResourceGoal::LocateResource() {
	if (GetSituation().OnSurface()) {
		const world::TileType &t = GetSituation().GetTileType();
		auto yield = t.FindResource(res);
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
	glm::ivec2 end(glm::min(glm::ivec2(planet.SideLength() - 1), loc + seek_radius));

	const world::TileType::Yield *best = nullptr;
	glm::ivec2 best_pos;
	double best_distance;

	for (int y = begin.y; y <= end.y; ++y) {
		for (int x = begin.x; x <= end.x; ++x) {
			const world::TileType &type = planet.TypeAt(srf, x, y);
			auto yield = type.FindResource(res);
			if (yield != type.resources.cend()) {
				double dist = glm::length2(planet.TileCenter(srf, x, y) - pos);
				if (!best) {
					best = &*yield;
					best_pos = glm::ivec2(x, y);
					best_distance = dist;
				} else if (yield->ubiquity - (dist * 0.125) > best->ubiquity - (best_distance * 0.125)) {
					best = &*yield;
					best_pos = glm::ivec2(x, y);
					best_distance = dist;
				}
			}
		}
	}
	if (best) {
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
