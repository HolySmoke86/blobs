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

};

}
}

#endif
