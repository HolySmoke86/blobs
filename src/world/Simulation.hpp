#ifndef BLOBS_WORLD_SIMULATION_HPP_
#define BLOBS_WORLD_SIMULATION_HPP_

#include "Set.hpp"

#include <set>


namespace blobs {
namespace world {

class Body;
class Planet;
class Resource;
class Sun;
class TileType;

class Simulation {

public:
	explicit Simulation(Body &root, const Set<Resource> &, const Set<TileType> &);
	~Simulation();

	Simulation(const Simulation &) = delete;
	Simulation &operator =(const Simulation &) = delete;

	Simulation(Simulation &&) = delete;
	Simulation &operator =(Simulation &&) = delete;

public:
	void Tick();

	void AddBody(Body &);
	void AddPlanet(Planet &);
	void AddSun(Sun &);

	Body &Root() noexcept { return root; }
	const Body &Root() const noexcept { return root; }

	const Set<Resource> &Resources() const noexcept { return resources; }
	const Set<TileType> &TileTypes() const noexcept { return tile_types; }

	const std::set<Body *> &Bodies() const noexcept { return bodies; }
	const std::set<Planet *> &Planets() const noexcept { return planets; }
	const std::set<Sun *> &Suns() const noexcept { return suns; }

	double Time() const noexcept { return time; }

private:
	Body &root;
	const Set<Resource> &resources;
	const Set<TileType> &tile_types;
	std::set<Body *> bodies;
	std::set<Planet *> planets;
	std::set<Sun *> suns;
	double time;

};

}
}

#endif
