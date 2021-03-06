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
		// position
		glm::dvec3 pos;
		// velocity
		glm::dvec3 vel;
		// face direction, normalized
		glm::dvec3 dir;
		State(
			const glm::dvec3 &pos = glm::dvec3(0.0),
			const glm::dvec3 &vel = glm::dvec3(0.0),
			const glm::dvec3 &dir = glm::dvec3(0.0, 0.0, -1.0))
		: pos(pos), vel(vel), dir(dir) { }
	};
	struct Derivative {
		// velocity
		glm::dvec3 vel;
		// acceleration
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
	bool OnGround() const noexcept;
	const glm::dvec3 &Position() const noexcept { return state.pos; }
	glm::dvec3 SurfaceNormal() const noexcept;
	world::Tile &GetTile() const noexcept;
	const world::TileType &GetTileType() const noexcept;

	void SetState(const State &s) noexcept { state = s; }
	const State &GetState() const noexcept { return state; }

	const glm::dvec3 &Velocity() const noexcept { return state.vel; }
	bool Moving() const noexcept { return glm::length2(state.vel) > 0.00001; }
	void Move(const glm::dvec3 &dp) noexcept;
	void Accelerate(const glm::dvec3 &dv) noexcept;
	void EnforceConstraints(State &) const noexcept;

	void Heading(const glm::dvec3 &h) noexcept { state.dir = h; }
	const glm::dvec3 &Heading() const noexcept { return state.dir; }

	void SetPlanetSurface(world::Planet &, const glm::dvec3 &pos) noexcept;

public:
	world::Planet *planet;
	State state;
	enum {
		LOST,
		PLANET_SURFACE,
	} type;

};

}
}

#endif
