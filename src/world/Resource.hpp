#ifndef BLOBS_WORLD_RESOURCE_HPP_
#define BLOBS_WORLD_RESOURCE_HPP_

#include "../math/glm.hpp"

#include <string>


namespace blobs {
namespace world {

struct Resource {

	std::string name = "";
	std::string label = "";

	double density = 1.0;
	double energy = 1.0;
	double inverse_energy = 1.0;

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

	glm::dvec3 base_color = glm::dvec3(1.0);

};

}
}

#endif
