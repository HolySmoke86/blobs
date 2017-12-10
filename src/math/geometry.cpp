#include "geometry.hpp"


namespace blobs {
namespace math {

bool Intersect(
	const AABB &a_box,
	const glm::dmat4 &a_m,
	const AABB &b_box,
	const glm::dmat4 &b_m,
	glm::dvec3 &normal,
	double &depth
) noexcept {
	glm::dvec3 a_corners[8] = {
		glm::dvec3(a_m * glm::dvec4(a_box.min.x, a_box.min.y, a_box.min.z, 1.0)),
		glm::dvec3(a_m * glm::dvec4(a_box.min.x, a_box.min.y, a_box.max.z, 1.0)),
		glm::dvec3(a_m * glm::dvec4(a_box.min.x, a_box.max.y, a_box.min.z, 1.0)),
		glm::dvec3(a_m * glm::dvec4(a_box.min.x, a_box.max.y, a_box.max.z, 1.0)),
		glm::dvec3(a_m * glm::dvec4(a_box.max.x, a_box.min.y, a_box.min.z, 1.0)),
		glm::dvec3(a_m * glm::dvec4(a_box.max.x, a_box.min.y, a_box.max.z, 1.0)),
		glm::dvec3(a_m * glm::dvec4(a_box.max.x, a_box.max.y, a_box.min.z, 1.0)),
		glm::dvec3(a_m * glm::dvec4(a_box.max.x, a_box.max.y, a_box.max.z, 1.0)),
	};

	glm::dvec3 b_corners[8] = {
		glm::dvec3(b_m * glm::dvec4(b_box.min.x, b_box.min.y, b_box.min.z, 1.0)),
		glm::dvec3(b_m * glm::dvec4(b_box.min.x, b_box.min.y, b_box.max.z, 1.0)),
		glm::dvec3(b_m * glm::dvec4(b_box.min.x, b_box.max.y, b_box.min.z, 1.0)),
		glm::dvec3(b_m * glm::dvec4(b_box.min.x, b_box.max.y, b_box.max.z, 1.0)),
		glm::dvec3(b_m * glm::dvec4(b_box.max.x, b_box.min.y, b_box.min.z, 1.0)),
		glm::dvec3(b_m * glm::dvec4(b_box.max.x, b_box.min.y, b_box.max.z, 1.0)),
		glm::dvec3(b_m * glm::dvec4(b_box.max.x, b_box.max.y, b_box.min.z, 1.0)),
		glm::dvec3(b_m * glm::dvec4(b_box.max.x, b_box.max.y, b_box.max.z, 1.0)),
	};

	glm::dvec3 axes[15] = {
		glm::dvec3(a_m[0]),
		glm::dvec3(a_m[1]),
		glm::dvec3(a_m[2]),
		glm::dvec3(b_m[0]),
		glm::dvec3(b_m[1]),
		glm::dvec3(b_m[2]),
		glm::normalize(glm::cross(glm::dvec3(a_m[0]), glm::dvec3(b_m[0]))),
		glm::normalize(glm::cross(glm::dvec3(a_m[0]), glm::dvec3(b_m[1]))),
		glm::normalize(glm::cross(glm::dvec3(a_m[0]), glm::dvec3(b_m[2]))),
		glm::normalize(glm::cross(glm::dvec3(a_m[1]), glm::dvec3(b_m[0]))),
		glm::normalize(glm::cross(glm::dvec3(a_m[1]), glm::dvec3(b_m[1]))),
		glm::normalize(glm::cross(glm::dvec3(a_m[1]), glm::dvec3(b_m[2]))),
		glm::normalize(glm::cross(glm::dvec3(a_m[2]), glm::dvec3(b_m[0]))),
		glm::normalize(glm::cross(glm::dvec3(a_m[2]), glm::dvec3(b_m[1]))),
		glm::normalize(glm::cross(glm::dvec3(a_m[2]), glm::dvec3(b_m[2]))),
	};

	depth = std::numeric_limits<double>::infinity();
	int min_axis = 0;

	int cur_axis = 0;
	for (const glm::dvec3 &axis : axes) {
		if (glm::any(glm::isnan(axis))) {
			// can result from the cross products if A and B have parallel axes
			++cur_axis;
			continue;
		}
		double a_min = std::numeric_limits<double>::infinity();
		double a_max = -std::numeric_limits<double>::infinity();
		for (const glm::dvec3 &corner : a_corners) {
			double val = glm::dot(corner, axis);
			a_min = std::min(a_min, val);
			a_max = std::max(a_max, val);
		}

		double b_min = std::numeric_limits<double>::infinity();
		double b_max = -std::numeric_limits<double>::infinity();
		for (const glm::dvec3 &corner : b_corners) {
			double val = glm::dot(corner, axis);
			b_min = std::min(b_min, val);
			b_max = std::max(b_max, val);
		}

		if (a_max < b_min || b_max < a_min) return false;

		double overlap = std::min(a_max, b_max) - std::max(a_min, b_min);
		if (overlap < depth) {
			depth = overlap;
			min_axis = cur_axis;
		}

		++cur_axis;
	}

	normal = axes[min_axis];
	return true;
}


bool Intersect(
	const Ray &ray,
	const AABB &aabb,
	const glm::dmat4 &M,
	glm::dvec3 &normal,
	double &dist
) noexcept {
	double t_min = 0.0;
	double t_max = std::numeric_limits<double>::infinity();
	const glm::dvec3 aabb_pos(M[3].x, M[3].y, M[3].z);
	const glm::dvec3 delta = aabb_pos - ray.Origin();

	glm::dvec3 t1(t_min, t_min, t_min), t2(t_max, t_max, t_max);

	for (int i = 0; i < 3; ++i) {
		const glm::dvec3 axis(M[i].x, M[i].y, M[i].z);
		const double e = glm::dot(axis, delta);
		const double f = glm::dot(axis, ray.Direction());

		if (std::abs(f) > std::numeric_limits<double>::epsilon()) {
			t1[i] = (e + aabb.min[i]) / f;
			t2[i] = (e + aabb.max[i]) / f;

			t_min = std::max(t_min, std::min(t1[i], t2[i]));
			t_max = std::min(t_max, std::max(t1[i], t2[i]));

			if (t_max < t_min) {
				return false;
			}
		} else {
			if (aabb.min[i] - e > 0.0 || aabb.max[i] - e < 0.0) {
				return false;
			}
		}
	}

	dist = t_min;

	glm::dvec3 min_all(glm::min(t1, t2));
	if (min_all.x > min_all.y) {
		if (min_all.x > min_all.z) {
			normal = glm::dvec3(t2.x < t1.x ? 1 : -1, 0, 0);
		} else {
			normal = glm::dvec3(0, 0, t2.z < t1.z ? 1 : -1);
		}
	} else if (min_all.y > min_all.z) {
		normal = glm::dvec3(0, t2.y < t1.y ? 1 : -1, 0);
	} else {
		normal = glm::dvec3(0, 0, t2.z < t1.z ? 1 : -1);
	}

	return true;
}

bool Intersect(
	const Ray &r,
	const Sphere &s,
	glm::dvec3 &normal,
	double &dist
) noexcept {
	const glm::dvec3 diff(s.origin - r.Origin());
	if (glm::dot(diff, r.Direction()) < 0.0) {
		if (glm::length2(diff) > s.radius * s.radius) return false;
		if (std::abs(glm::length2(diff) - s.radius * s.radius) < std::numeric_limits<double>::epsilon() * s.radius) {
			normal = glm::normalize(-diff);
			dist = 0.0;
			return true;
		}
		const glm::dvec3 pc(r.Direction() * glm::dot(r.Direction(), diff) + r.Origin());
		double idist = std::sqrt(s.radius * s.radius - glm::length2(pc - s.origin));
		dist = idist - glm::length(pc - r.Origin());
		normal = glm::normalize((r.Origin() + (r.Direction() * dist)) - s.origin);
		return true;
	}
	const glm::dvec3 pc(r.Direction() * glm::dot(r.Direction(), diff) + r.Origin());
	if (glm::length2(s.origin - pc) > s.radius * s.radius) return false;
	double idist = std::sqrt(s.radius * s.radius - glm::length2(pc - s.origin));
	if (glm::length2(diff) > s.radius * s.radius) {
		dist = glm::length(pc - r.Origin()) - idist;
	} else {
		dist = glm::length(pc - r.Origin()) + idist;
	}
	normal = glm::normalize((r.Origin() + (r.Direction() * dist)) - s.origin);
	return true;
}

}
}
