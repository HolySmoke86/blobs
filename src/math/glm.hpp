#ifndef BLOBS_MATH_GLM_HPP_
#define BLOBS_MATH_GLM_HPP_

#ifndef GLM_FORCE_RADIANS
#  define GLM_FORCE_RADIANS 1
#endif

#include <limits>
#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtx/component_wise.hpp>

// GLM moved tvec[1234] from glm::detail to glm in 0.9.6

#if GLM_VERSION < 96

namespace glm {
	using tvec1 = detail::tvec1;
	using tvec2 = detail::tvec2;
	using tvec3 = detail::tvec3;
	using tvec4 = detail::tvec4;
}

#endif

template <class T>
inline bool allzero(const T &v) noexcept {
	return glm::length2(v) <
		std::numeric_limits<typename T::value_type>::epsilon()
		* std::numeric_limits<typename T::value_type>::epsilon();
}

template <class T>
inline bool anynan(const T &v) noexcept {
	return glm::any(glm::isnan(v));
}


#endif
