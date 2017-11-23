#include "Goal.hpp"
#include "LocateResourceGoal.hpp"

#include "Creature.hpp"
#include "../world/Planet.hpp"
#include "../world/TileType.hpp"

namespace blobs {
namespace creature {

Goal::Goal()
: urgency(0.0)
, interruptible(true)
, complete(false) {
}

Goal::~Goal() noexcept {
}


LocateResourceGoal::LocateResourceGoal(int res)
: res(res) {
}

LocateResourceGoal::~LocateResourceGoal() noexcept {
}

void LocateResourceGoal::Enable(Creature &c) {
}

void LocateResourceGoal::Tick(double dt) {
}

void LocateResourceGoal::Action(Creature &c) {
	if (c.GetSituation().OnSurface()) {
		const world::TileType &t = c.GetSituation().GetTileType();
		auto yield = t.FindResource(res);
		if (yield != t.resources.cend()) {
			// hoooray
			c.GetSteering().Halt();
			Complete(true);
		} else {
			// go find somewhere else
			const world::Planet &planet = c.GetSituation().GetPlanet();
			double side_length = planet.SideLength();
			double offset = side_length * 0.5;
			glm::ivec2 loc = glm::ivec2(c.GetSituation().Position() + offset);
			glm::ivec2 seek_radius(2);
			glm::ivec2 begin(glm::max(glm::ivec2(0), loc - seek_radius));
			glm::ivec2 end(glm::min(glm::ivec2(side_length), loc + seek_radius));
			const world::TileType::Yield *best = nullptr;
			glm::ivec2 best_pos;
			double best_distance = 2 * side_length * side_length;
			for (int y = begin.y; y < end.y; ++y) {
				for (int x = begin.x; x < end.x; ++x) {
					const world::TileType &type = planet.TypeAt(c.GetSituation().Surface(), x, y);
					auto yield = type.FindResource(res);
					if (yield != type.resources.cend()) {
						double dist = glm::length2(glm::dvec3(x - offset + 0.5, y - offset + 0.5, 0.0) - c.GetSituation().Position());
						if (!best) {
							best = &*yield;
							best_pos = glm::ivec2(x, y);
							best_distance = dist;
						} else if (yield->ubiquity + (dist * 0.125) > best->ubiquity + (best_distance * 0.125)) {
							best = &*yield;
							best_pos = glm::ivec2(x, y);
							best_distance = dist;
						}
					}
				}
			}
			if (best) {
				c.GetSteering().GoTo(glm::dvec3(best_pos.x - offset + 0.5, best_pos.y - offset + 0.5, 0.0));
			} else {
				// oh crap
			}
		}
	} else {
		// well, what now?
	}
}

}
}
