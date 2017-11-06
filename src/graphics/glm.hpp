#ifndef BLOBS_GRAPHICS_GLM_HPP_
#define BLOBS_GRAPHICS_GLM_HPP_

#ifndef GLM_FORCE_RADIANS
#  define GLM_FORCE_RADIANS 1
#endif

#include <glm/glm.hpp>

// GLM moved tvec[1234] from glm::detail to glm in 0.9.6

#if GLM_VERSION < 96

namespace glm {
	using tvec1 = detail::tvec1;
	using tvec2 = detail::tvec2;
	using tvec3 = detail::tvec3;
	using tvec4 = detail::tvec4;
}

#endif

#endif
