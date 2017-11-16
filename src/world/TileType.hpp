#ifndef BLOBS_WORLD_TILETYPE_HPP_
#define BLOBS_WORLD_TILETYPE_HPP_

#include <string>
#include <vector>


namespace blobs {
namespace world {

struct TileType {

	std::string name;
	std::string label;

	int id;
	int texture;

	struct Yield {
		int resource;
		double ubiquity;
	};
	std::vector<Yield> resources;

};

}
}

#endif
