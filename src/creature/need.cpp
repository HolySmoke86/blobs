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
	value = std::min(1.0, value + gain * dt);
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
, damage(damage) {
}

InhaleNeed::~InhaleNeed() {
}

void InhaleNeed::ApplyEffect(Creature &c, double dt) {
	if (!IsSatisfied()) {
		// TODO: make condition more natural with thresholds and stuff
		if (c.GetSituation().OnPlanet() && c.GetSituation().GetPlanet().Atmosphere() == resource) {
			value = std::max(0.0, value - (speed * dt));
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
