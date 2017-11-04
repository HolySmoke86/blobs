#ifndef BLOBS_WORLD_PLANET_HPP_
#define BLOBS_WORLD_PLANET_HPP_

#include "Tile.hpp"

#include <cassert>
#include <memory>


namespace blobs {
namespace world {

struct Tile;

/// A planet has six surfaces, numbered 0 to 5, each with tiles from
/// +radius to -radius.
class Planet {

public:
	explicit Planet(int radius);
	~Planet();

	Planet(Planet &&);
	Planet &operator =(Planet &&);

	Planet(const Planet &) = delete;
	Planet &operator =(const Planet &) = delete;

public:
	/// Get the tile at given surface and coordinates.
	Tile &TileAt(int surface, int x, int y) {
		return tiles[IndexOf(surface, x, y)];
	}
	const Tile &TileAt(int surface, int x, int y) const {
		return tiles[IndexOf(surface, x, y)];
	}

	/// Convert coordinates into a tile index.
	int IndexOf(int surface, int x, int y) const {
		assert(0 <= surface && surface <= 5);
		assert(-radius <= x && x <= radius);
		assert(-radius <= y && y <= radius);
		return surface * SurfaceArea() + ToOffset(y) * SideLength() + ToOffset(x);
	}
	/// Convert coordinate into offset
	int ToOffset(int c) const {
		return c + radius;
	}
	/// The "radius" of the planet.
	int Radius() const {
		return radius;
	}
	/// The length of the side of each surface.
	int SideLength() const {
		return 2 * radius + 1;
	}
	/// The area (or number of tiles) of one surface
	int SurfaceArea() const {
		return SideLength() * SideLength();
	}
	/// Total area of all surfaces combined.
	int TotalArea() const {
		return 6 * SurfaceArea();
	}

private:
	int radius;
	std::unique_ptr<Tile []> tiles;

};

void GenerateTest(Planet &);

}
}

#endif
