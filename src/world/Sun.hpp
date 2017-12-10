#ifndef BLOBS_WORLD_SUN_HPP_
#define BLOBS_WORLD_SUN_HPP_

#include "Body.hpp"


namespace blobs {
namespace world {

class Sun
: public Body {

public:
	Sun();
	~Sun();

	Sun(const Sun &) = delete;
	Sun &operator =(const Sun &) = delete;

	Sun(Sun &&) = delete;
	Sun &operator =(Sun &&) = delete;

public:
	void Color(const glm::dvec3 &c) noexcept { color = c; }
	const glm::dvec3 &Color() const noexcept { return color; }

	void Luminosity(double l) noexcept { luminosity = l; }
	double Luminosity() const noexcept { return luminosity; }

private:
	glm::dvec3 color;
	double luminosity;

};

}
}

#endif
