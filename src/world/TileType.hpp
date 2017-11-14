#ifndef BLOBS_WORLD_TILETYPE_HPP_
#define BLOBS_WORLD_TILETYPE_HPP_

#include <string>


namespace blobs {
namespace world {

struct TileType {

	std::string name;
	std::string label;

	int id;
	int texture;

};

}
}

#endif
