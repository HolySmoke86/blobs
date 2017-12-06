#ifndef BLOBS_WORLD_PLANET_HPP_
#define BLOBS_WORLD_PLANET_HPP_

#include "Body.hpp"

#include "Set.hpp"
#include "Tile.hpp"
#include "../graphics/SimpleVAO.hpp"
#include "../math/glm.hpp"

#include <cassert>
#include <memory>
#include <vector>
#include <GL/glew.h>


namespace blobs {
namespace world {

class TileType;

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
	/// surface normal
	glm::dvec3 NormalAt(const glm::dvec3 &p) const noexcept { return normalize(p); }
	/// height over surface
	double ElevationAt(const glm::dvec3 &p) const noexcept { return length(p) - Radius(); }
	/// distance to planet center
	double DistanceAt(const glm::dvec3 &p) const noexcept { return length(p); }

	/// get ground tile
	Tile &TileAt(const glm::dvec3 &) noexcept;
	const Tile &TileAt(const glm::dvec3 &) const noexcept;
	const TileType &TileTypeAt(const glm::dvec3 &) const noexcept;

	/// Get the tile at given surface and coordinates.
	Tile &TileAt(int surface, int x, int y) noexcept;
	const Tile &TileAt(int surface, int x, int y) const noexcept;
	const TileType &TypeAt(int surface, int x, int y) const noexcept;

	/// The length in tiles of the side of each surface.
	int SideLength() const { return sidelength; }

	// center point of tile on surface at elevation
	glm::dvec3 TileCenter(int surface, int x, int y, double elevation = 0.0) const noexcept;

	void BuildVAO(const Set<TileType> &);
	void Draw(app::Assets &, graphics::Viewport &) override;

private:
	/// Convert coordinates into a tile index.
	int IndexOf(int surface, int x, int y) const {
		assert(0 <= surface && surface <= 5);
		assert(0 <= x && x < sidelength);
		assert(0 <= y && y < sidelength);
		return surface * TilesPerSurface() + y * SideLength() + x;
	}
	/// The number of tiles of one surface.
	int TilesPerSurface() const {
		return SideLength() * SideLength();
	}
	/// Total number of tiles of all surfaces combined.
	int TilesTotal() const {
		return 6 * TilesPerSurface();
	}

private:
	int sidelength;
	std::vector<Tile> tiles;

	struct Attributes {
		glm::vec3 position;
		glm::vec3 normal;
		glm::vec3 tex_coord;
	};
	std::unique_ptr<graphics::SimpleVAO<Attributes, unsigned int>> vao;

};

void GenerateEarthlike(const Set<TileType> &, Planet &) noexcept;
void GenerateTest(const Set<TileType> &, Planet &) noexcept;

}
}

#endif
