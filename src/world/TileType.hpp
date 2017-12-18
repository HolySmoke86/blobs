#ifndef BLOBS_WORLD_TILETYPE_HPP_
#define BLOBS_WORLD_TILETYPE_HPP_

#include <string>
#include <vector>


namespace blobs {
namespace creature {
	class Composition;
}
namespace world {

struct TileType {

	std::string name;
	std::string label;

	int id;
	int texture;

	double shiny = 5.0;
	double glossy = 0.5;
	double metallic = 0.0;

	struct Yield {
		int resource;
		double ubiquity;
	};
	std::vector<Yield> resources;

	std::vector<Yield>::const_iterator FindResource(int) const;
	std::vector<Yield>::const_iterator FindBestResource(const creature::Composition &) const;

};

}
}

#endif
