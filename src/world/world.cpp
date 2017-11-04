#include "Planet.hpp"
#include "Tile.hpp"

#include <algorithm>


namespace blobs {
namespace world {

Planet::Planet(int radius)
: radius(radius)
, tiles(new Tile[TotalArea()]) {

}

Planet::~Planet() {
}

Planet::Planet(Planet &&other)
: radius(other.radius)
, tiles(other.tiles.release()) {
}

Planet &Planet::operator =(Planet &&other) {
	radius = other.radius;
	std::swap(tiles, other.tiles);
	return *this;
}


void GenerateTest(Planet &p) {
	for (int surface = 0; surface <= 5; ++surface) {
		for (int y = -p.Radius(); y <= p.Radius(); ++y) {
			for (int x = -p.Radius(); x <= p.Radius(); ++x) {
				p.TileAt(surface, x, y).type = (x == 0) + (y == 0);
			}
		}
	}
}

}
}
