#ifndef BLOBS_MATH_GLM_HPP_
#define BLOBS_MATH_GLM_HPP_

#ifndef GLM_FORCE_RADIANS
#  define GLM_FORCE_RADIANS 1
#endif

#include <algorithm>
#include <limits>
#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtx/component_wise.hpp>

// GLM moved tvec[1234] from glm::detail to glm in 0.9.6

#if GLM_VERSION < 96

namespace glm {
	template<class T, precision P = defaultp>
	using tvec1 = detail::tvec1<T, P>;
	template<class T, precision P = defaultp>
	using tvec2 = detail::tvec2<T, P>;
	template<class T, precision P = defaultp>
	using tvec3 = detail::tvec3<T, P>;
	template<class T, precision P = defaultp>
	using tvec4 = detail::tvec4<T, P>;
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

template<class Vec>
inline Vec limit(const Vec &v, typename Vec::value_type max) noexcept {
	typename Vec::value_type len2 = glm::length2(v);
	typename Vec::value_type max2 = max * max;
	if (len2 > max2) {
		return glm::normalize(v) * max;
	} else {
		return v;
	}
}

template<class T>
inline T rgb2hsl(const T &rgb) {
	using Vec4 = glm::tvec4<typename T::value_type>;
	const Vec4 K(0.0, -1.0/3.0, 2.0/3.0, -1.0);
	const Vec4 p(glm::mix(Vec4(rgb.z, rgb.y, K.w, K.z), Vec4(rgb.y, rgb.z, K.x, K.y), rgb.y < rgb.z ? 0.0 : 1.0));
	const Vec4 q(glm::mix(Vec4(p.x, p.y, p.w, rgb.x), Vec4(rgb.x, p.y, p.z, p.x), rgb.x < p.x ? 0.0 : 1.0));
	const typename T::value_type d = q.x - std::min(q.w, q.y);
	const typename T::value_type e = 1.0e-10;
	T hsl = rgb;
	hsl.x = std::abs(q.z + (q.w - q.y) / (6.0 * d + e));
	hsl.y = d / (q.x + e);
	hsl.z = q.x;
	return hsl;
}

template<class T>
inline T hsl2rgb(const T &hsl) {
	using Vec3 = glm::tvec3<typename T::value_type>;
	using Vec4 = glm::tvec4<typename T::value_type>;
	const Vec4 K(1.0, 2.0/3.0, 1.0/3.0, 3.0);
	const Vec3 p(glm::abs(glm::fract(Vec3(hsl.x) + Vec3(K)) * 6.0 - Vec3(K.w)));
	T rgb = hsl.z * glm::mix(Vec3(K.x), glm::clamp(p - Vec3(K.x), 0.0, 1.0), hsl.y);
	return rgb;
}

#endif
