#ifndef BLOBS_WORLD_PLANET_HPP_
#define BLOBS_WORLD_PLANET_HPP_

#include "Body.hpp"

#include "Tile.hpp"
#include "../graphics/glm.hpp"
#include "../graphics/SimpleVAO.hpp"

#include <cassert>
#include <vector>
#include <GL/glew.h>


namespace blobs {
namespace world {

class TileSet;

/// A planet has six surfaces, numbered 0 to 5, each filled with
/// sidelength² tiles.
class Planet
: public Body {

public:
	explicit Planet(int sidelength);
	~Planet();

	Planet(const Planet &) = delete;
	Planet &operator =(const Planet &) = delete;

	Planet(Planet &&) = delete;
	Planet &operator =(Planet &&) = delete;

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
		assert(0 <= x && x <= sidelength);
		assert(0 <= y && y <= sidelength);
		return surface * TilesPerSurface() + y * SideLength() + x;
	}
	/// The length of the side of each surface.
	int SideLength() const {
		return sidelength;
	}
	/// The number of tiles of one surface.
	int TilesPerSurface() const {
		return SideLength() * SideLength();
	}
	/// Total number of tiles of all surfaces combined.
	int TilesTotal() const {
		return 6 * TilesPerSurface();
	}

	glm::dvec3 TileCenter(int surface, int x, int y) const noexcept;

	void BuildVAO(const TileSet &);
	void Draw(app::Assets &, graphics::Viewport &) override;

private:
	int sidelength;
	std::vector<Tile> tiles;

	struct Attributes {
		glm::vec3 position;
		glm::vec3 tex_coord;
	};
	graphics::SimpleVAO<Attributes, unsigned int> vao;

};

void GenerateEarthlike(const TileSet &, Planet &) noexcept;
void GenerateTest(const TileSet &, Planet &) noexcept;

}
}

#endif
