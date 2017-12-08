#ifndef BLOBS_WORLD_CREATURECREATURECOLLISION_HPP_
#define BLOBS_WORLD_CREATURECREATURECOLLISION_HPP_

#include "../math/glm.hpp"


namespace blobs {
namespace creature {
	class Creature;
}
namespace world {

class CreatureCreatureCollision {

public:
	CreatureCreatureCollision(
		creature::Creature &a,
		creature::Creature &b,
		const glm::dvec3 &n,
		double depth
	) : a(&a), b(&b), normal(n), depth(depth) {
		if (glm::dot(normal, BPos() - APos()) < 0.0) {
			// make sure normal always is in direction from A to B
			normal *= -1.0;
		}
	}
	~CreatureCreatureCollision();

public:
	creature::Creature &A() noexcept { return *a; }
	const creature::Creature &A() const noexcept { return *a; }
	const glm::dvec3 &APos() const noexcept;
	const glm::dvec3 &AVel() const noexcept;

	creature::Creature &B() noexcept { return *b; }
	const creature::Creature &B() const noexcept { return *b; }
	const glm::dvec3 &BPos() const noexcept;
	const glm::dvec3 &BVel() const noexcept;

	const glm::dvec3 &Normal() const noexcept { return normal; }
	double Depth() const noexcept { return depth; }

private:
	creature::Creature *a;
	creature::Creature *b;
	glm::dvec3 normal;
	double depth;

};

}
}

#endif
