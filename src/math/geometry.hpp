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

}
}

#endif
