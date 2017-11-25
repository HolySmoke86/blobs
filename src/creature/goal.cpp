#include "Goal.hpp"
#include "LocateResourceGoal.hpp"

#include "Creature.hpp"
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


LocateResourceGoal::LocateResourceGoal(Creature &c, int res)
: Goal(c)
, res(res)
, found(false)
, target_pos(0.0)
, target_srf(0)
, target_tile(0) {
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
		LocateResource();
	} else if (OnTargetTile()) {
		GetSteering().Halt();
		if (!GetCreature().Moving()) {
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
	glm::ivec2 end(glm::min(glm::ivec2(planet.SideLength()), loc + seek_radius));

	const world::TileType::Yield *best = nullptr;
	glm::ivec2 best_pos;
	double best_distance;

	for (int y = begin.y; y < end.y; ++y) {
		for (int x = begin.x; x < end.x; ++x) {
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
		target_pos = planet.TileCenter(srf, best_pos.x, best_pos.y);
		target_srf = srf;
		target_tile = best_pos;
		GetSteering().GoTo(target_pos);
		std::cout << "found resource at " << target_pos << std::endl;
	} else {
		// oh crap
	}
}

bool LocateResourceGoal::OnTargetTile() const noexcept {
	const Situation &s = GetSituation();
	return s.OnSurface()
		&& s.Surface() == target_srf
		&& s.GetPlanet().SurfacePosition(s.Surface(), s.Position()) == target_tile;
}

}
}