#ifndef BLOBS_MATH_GEOMETRY_HPP_
#define BLOBS_MATH_GEOMETRY_HPP_

#include "glm.hpp"

#include <algorithm>


namespace blobs {
namespace math {

struct AABB {

	glm::dvec3 min;
	glm::dvec3 max;

	void Adjust() noexcept {
		if (max.x < min.x) std::swap(max.x, min.x);
		if (max.y < min.y) std::swap(max.y, min.y);
		if (max.z < min.z) std::swap(max.z, min.z);
	}

	glm::dvec3 Center() const noexcept {
		return min + (max - min) * 0.5;
	}

};

/// matrices must not scale
bool Intersect(
	const AABB &a_box,
	const glm::dmat4 &a_m,
	const AABB &b_box,
	const glm::dmat4 &b_m,
	glm::dvec3 &normal,
	double &depth) noexcept;

class Ray {

public:
	Ray(const glm::dvec3 &orig, const glm::dvec3 &dir)
	: orig(orig), dir(dir), inv_dir(1.0 / dir) { }

	void Origin(const glm::dvec3 &o) noexcept { orig = o; }
	const glm::dvec3 &Origin() const noexcept { return orig; }
	void Direction(const glm::dvec3 &d) noexcept { dir = d; inv_dir = 1.0 / d; }
	const glm::dvec3 &Direction() const noexcept { return dir; }
	const glm::dvec3 &InverseDirection() const noexcept { return inv_dir; }

private:
	glm::dvec3 orig;
	glm::dvec3 dir;
	glm::dvec3 inv_dir;

};

/// oriented ray/box intersection test
bool Intersect(
	const Ray &,
	const AABB &,
	const glm::dmat4 &M,
	glm::dvec3 &normal,
	double &dist) noexcept;

}
}

#endif
