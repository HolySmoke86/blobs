#include "Need.hpp"
#include "InhaleNeed.hpp"
#include "IngestNeed.hpp"

#include "Creature.hpp"
#include "../world/Planet.hpp"


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
, damage(damage) {
}

IngestNeed::~IngestNeed() {
}

void IngestNeed::ApplyEffect(Creature &c, double dt) {
	if (!IsSatisfied()) {
		// TODO: find resource and start ingest task
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
	if (!IsSatisfied() && !inhaling) {
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
