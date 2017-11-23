#include "Need.hpp"
#include "InhaleNeed.hpp"
#include "IngestNeed.hpp"

#include "Creature.hpp"
#include "LocateResourceGoal.hpp"
#include "../world/Planet.hpp"
#include "../world/TileType.hpp"


namespace blobs {
namespace creature {

Need::~Need() {
}

void Need::Tick(double dt) noexcept {
	Increase(gain * dt);
}

void Need::Increase(double delta) noexcept {
	value = std::min(1.0, value + delta);
}

void Need::Decrease(double delta) noexcept {
	value = std::max(0.0, value - delta);
}


IngestNeed::IngestNeed(int resource, double speed, double damage)
: resource(resource)
, speed(speed)
, damage(damage)
, ingesting(false)
, locating(false) {
}

IngestNeed::~IngestNeed() {
}

void IngestNeed::ApplyEffect(Creature &c, double dt) {
	if (!IsSatisfied()) {
		ingesting = true;
	}
	if (ingesting) {
		if (c.GetSituation().OnSurface()) {
			const world::TileType &t = c.GetSituation().GetTileType();
			bool found = false;
			for (auto &yield : t.resources) {
				if (yield.resource == resource) {
					found = true;
					// TODO: check if not busy with something else
					Decrease(std::min(yield.ubiquity, speed) * dt);
					if (value == 0.0) {
						ingesting = false;
						locating = false;
					}
					break;
				}
			}
			if (!found && !locating) {
				c.AddGoal(std::unique_ptr<Goal>(new LocateResourceGoal(resource)));
				locating = true;
			}
		}
	}
	if (IsCritical()) {
		c.Hurt(damage * dt);
	}
}


InhaleNeed::InhaleNeed(int resource, double speed, double damage)
: resource(resource)
, speed(speed)
, damage(damage)
, inhaling(false) {
}

InhaleNeed::~InhaleNeed() {
}

void InhaleNeed::ApplyEffect(Creature &c, double dt) {
	if (!IsSatisfied()) {
		inhaling = true;
	}
	if (inhaling) {
		if (c.GetSituation().OnPlanet() && c.GetSituation().GetPlanet().Atmosphere() == resource) {
			Decrease(speed * dt);
			if (value == 0.0) {
				inhaling = false;
			}
		} else {
			// TODO: panic
		}
	}
	if (IsCritical()) {
		c.Hurt(damage * dt);
	}
}

}
}
