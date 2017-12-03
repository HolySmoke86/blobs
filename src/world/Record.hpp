#ifndef BLOBS_WORLD_RECORD_HPP_
#define BLOBS_WORLD_RECORD_HPP_

#include <string>


namespace blobs {
namespace creature {
	class Creature;
}
namespace world {

struct Record {

	std::string name = "";
	creature::Creature *holder = nullptr;
	double value = 0.0;
	double time = 0.0;
	enum Type {
		VALUE,
		LENGTH,
		MASS,
		PERCENTAGE,
		TIME,
	} type = VALUE;

	operator bool() const noexcept { return holder; }

};

}
}

#endif
