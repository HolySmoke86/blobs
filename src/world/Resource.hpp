#ifndef BLOBS_WORLD_RESOURCE_HPP_
#define BLOBS_WORLD_RESOURCE_HPP_

#include <string>


namespace blobs {
namespace world {

struct Resource {

	std::string name;
	std::string label;

	double density = 1.0;

	int id = -1;

	enum State {
		SOLID = 0,
		LIQUID = 1,
		GAS = 2,
		PLASMA = 3,
	};
	// the resource's natural state
	// TODO: something about temperature and pressure and stuff
	int state = SOLID;

};

}
}

#endif
