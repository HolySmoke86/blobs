#ifndef BLOBS_MATH_WORLEYNOISE_HPP_
#define BLOBS_MATH_WORLEYNOISE_HPP_

#include "glm.hpp"


namespace blobs {
namespace math {

/// implementation of Worley noise (aka Cell or Voroni noise)
class WorleyNoise {

public:
	explicit WorleyNoise(unsigned int seed) noexcept;

	float operator ()(const glm::vec3 &) const noexcept;

private:
	const unsigned int seed;
	const int num_points;

};

}
}

#endif
