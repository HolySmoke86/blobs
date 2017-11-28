#ifndef BLOBS_CREATURE_SITUATION_HPP_
#define BLOBS_CREATURE_SITUATION_HPP_

#include "../math/glm.hpp"


namespace blobs {
namespace world {
	class Planet;
	class Tile;
	class TileType;
}
namespace creature {

class Situation {

public:
	struct State {
		glm::dvec3 pos;
		glm::dvec3 vel;
		State(
			const glm::dvec3 &pos = glm::dvec3(0.0),
			const glm::dvec3 &vel = glm::dvec3(0.0))
		: pos(pos), vel(vel) { }
	};
	struct Derivative {
		glm::dvec3 vel;
		glm::dvec3 acc;
		Derivative(
			const glm::dvec3 &vel = glm::dvec3(0.0),
			const glm::dvec3 &acc = glm::dvec3(0.0))
		: vel(vel), acc(acc) { }
	};

public:
	Situation();
	~Situation();

	Situation(const Situation &) = delete;
	Situation &operator =(const Situation &) = delete;

	Situation(Situation &&) = delete;
	Situation &operator =(Situation &&) = delete;

public:
	bool OnPlanet() const noexcept;
	world::Planet &GetPlanet() const noexcept { return *planet; }
	bool OnSurface() const noexcept;
	int Surface() const noexcept { return surface; }
	const glm::dvec3 &Position() const noexcept { return state.pos; }
	bool OnTile() const noexcept;
	glm::ivec2 SurfacePosition() const noexcept;
	world::Tile &GetTile() const noexcept;
	const world::TileType &GetTileType() const noexcept;

	void SetState(const State &s) noexcept { state = s; }
	const State &GetState() const noexcept { return state; }

	const glm::dvec3 &Velocity() const noexcept { return state.vel; }
	bool Moving() const noexcept { return glm::length2(state.vel) < 0.00000001; }
	void Move(const glm::dvec3 &dp) noexcept;
	void SetPlanetSurface(world::Planet &, int srf, const glm::dvec3 &pos) noexcept;

public:
	world::Planet *planet;
	State state;
	int surface;
	enum {
		LOST,
		PLANET_SURFACE,
	} type;

};

}
}

#endif
