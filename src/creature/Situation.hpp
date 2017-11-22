#ifndef BLOBS_CREATURE_SITUATION_HPP_
#define BLOBS_CREATURE_SITUATION_HPP_

#include "../graphics/glm.hpp"


namespace blobs {
namespace world {
	class Planet;
	class Tile;
	class TileType;
}
namespace creature {

class Situation {

public:
	Situation();
	~Situation();

public:
	bool OnPlanet() const noexcept;
	world::Planet &GetPlanet() const noexcept { return *planet; }
	bool OnSurface() const noexcept;
	int Surface() const noexcept { return surface; }
	const glm::dvec3 &Position() const noexcept { return position; }
	world::Tile &GetTile() const noexcept;
	const world::TileType &GetTileType() const noexcept;

	void SetPlanetSurface(world::Planet &, int srf, const glm::dvec3 &pos) noexcept;

public:
	world::Planet *planet;
	glm::dvec3 position;
	int surface;
	enum {
		LOST,
		PLANET_SURFACE,
	} type;

};

}
}

#endif