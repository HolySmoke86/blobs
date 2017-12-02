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
		normalize(cross(glm::dvec3(a_m[0]), glm::dvec3(b_m[0]))),
		normalize(cross(glm::dvec3(a_m[0]), glm::dvec3(b_m[1]))),
		normalize(cross(glm::dvec3(a_m[0]), glm::dvec3(b_m[2]))),
		normalize(cross(glm::dvec3(a_m[1]), glm::dvec3(b_m[0]))),
		normalize(cross(glm::dvec3(a_m[1]), glm::dvec3(b_m[1]))),
		normalize(cross(glm::dvec3(a_m[1]), glm::dvec3(b_m[2]))),
		normalize(cross(glm::dvec3(a_m[2]), glm::dvec3(b_m[0]))),
		normalize(cross(glm::dvec3(a_m[2]), glm::dvec3(b_m[1]))),
		normalize(cross(glm::dvec3(a_m[2]), glm::dvec3(b_m[2]))),
	};

	depth = std::numeric_limits<double>::infinity();
	int min_axis = 0;

	int cur_axis = 0;
	for (const glm::dvec3 &axis : axes) {
		if (any(isnan(axis))) {
			// can result from the cross products if A and B have parallel axes
			++cur_axis;
			continue;
		}
		double a_min = std::numeric_limits<double>::infinity();
		double a_max = -std::numeric_limits<double>::infinity();
		for (const glm::dvec3 &corner : a_corners) {
			double val = dot(corner, axis);
			a_min = std::min(a_min, val);
			a_max = std::max(a_max, val);
		}

		double b_min = std::numeric_limits<double>::infinity();
		double b_max = -std::numeric_limits<double>::infinity();
		for (const glm::dvec3 &corner : b_corners) {
			double val = dot(corner, axis);
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


}
}
