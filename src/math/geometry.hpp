#ifndef BLOBS_MATH_GEOMETRY_HPP_
#define BLOBS_MATH_GEOMETRY_HPP_

#include "glm.hpp"

#include <algorithm>
#include <ostream>
#include <glm/gtx/io.hpp>


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

inline std::ostream &operator <<(std::ostream &out, const AABB &b) {
	return out << "AABB(" << b.min << ", " << b.max << ")";
}
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

inline Ray operator *(const glm::dmat4 &m, const Ray &r) noexcept {
	glm::dvec4 o(m * glm::dvec4(r.Origin(), 1.0));
	glm::dvec4 d(m * glm::dvec4(r.Origin() + r.Direction(), 1.0));
	return Ray(glm::dvec3(o) / o.w, glm::normalize((glm::dvec3(d) / d.w) - (glm::dvec3(o) / o.w)));
}

inline std::ostream &operator <<(std::ostream &out, const Ray &r) {
	return out << "Ray(" << r.Origin() << ", " << r.Direction() << ")";
}

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
