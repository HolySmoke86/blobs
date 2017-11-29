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
	/// Get the tile at given surface and coordinates.
	Tile &TileAt(int surface, int x, int y) {
		return tiles[IndexOf(surface, x, y)];
	}
	const Tile &TileAt(int surface, int x, int y) const {
		return tiles[IndexOf(surface, x, y)];
	}

	const TileType &TypeAt(int surface, int x, int y) const;

	/// Convert coordinates into a tile index.
	int IndexOf(int surface, int x, int y) const {
		assert(0 <= surface && surface <= 5);
		assert(0 <= x && x < sidelength);
		assert(0 <= y && y < sidelength);
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

	double TileToPosition(int t) const noexcept { return double(t) - Radius(); }
	int PositionToTile(double p) const noexcept { return int(p + Radius()); }

	// tile coordinates of position on surface
	glm::ivec2 SurfacePosition(int surface, const glm::dvec3 &) const noexcept;
	// height of point over surface
	double SurfaceElevation(int surface, const glm::dvec3 &) const noexcept;
	// center point of tile on surface at elevation
	glm::dvec3 TileCenter(int surface, int x, int y, double elevation = 0.0) const noexcept;

	static glm::dvec3 SurfaceNormal(int srf) noexcept {
		glm::dvec3 nrm(0.0);
		nrm[(srf + 2) % 3] = srf < 3 ? 1.0 : -1.0;
		return nrm;
	}
	static glm::dmat3 SurfaceOrientation(int srf) noexcept {
		glm::dmat3 mat(0.0);
		mat[(srf + 0) % 3][0] = 1.0;
		mat[(srf + 2) % 3][1] = srf < 3 ? 1.0 : -1.0;
		mat[(srf + 1) % 3][2] = srf < 3 ? 1.0 : -1.0;
		return mat;
	}

	void BuildVAO(const Set<TileType> &);
	void Draw(app::Assets &, graphics::Viewport &) override;

private:
	int sidelength;
	std::vector<Tile> tiles;

	struct Attributes {
		glm::vec3 position;
		glm::vec3 tex_coord;
	};
	std::unique_ptr<graphics::SimpleVAO<Attributes, unsigned int>> vao;

};

void GenerateEarthlike(const Set<TileType> &, Planet &) noexcept;
void GenerateTest(const Set<TileType> &, Planet &) noexcept;

}
}

#endif
