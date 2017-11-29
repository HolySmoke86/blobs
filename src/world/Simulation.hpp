#ifndef BLOBS_WORLD_SIMULATION_HPP_
#define BLOBS_WORLD_SIMULATION_HPP_

#include "Set.hpp"
#include "../app/Assets.hpp"

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
	explicit Simulation(Body &root, app::Assets &);
	~Simulation();

	Simulation(const Simulation &) = delete;
	Simulation &operator =(const Simulation &) = delete;

	Simulation(Simulation &&) = delete;
	Simulation &operator =(Simulation &&) = delete;

public:
	void Tick(double dt);

	void AddBody(Body &);
	void AddPlanet(Planet &);
	void AddSun(Sun &);

	Body &Root() noexcept { return root; }
	const Body &Root() const noexcept { return root; }

	app::Assets &Assets() noexcept { return assets; }
	const app::Assets &Assets() const noexcept { return assets; }
	const Set<Resource> &Resources() const noexcept { return assets.data.resources; }
	const Set<TileType> &TileTypes() const noexcept { return assets.data.tile_types; }

	const std::set<Body *> &Bodies() const noexcept { return bodies; }
	const std::set<Planet *> &Planets() const noexcept { return planets; }
	const std::set<Sun *> &Suns() const noexcept { return suns; }

	double Time() const noexcept { return time; }

private:
	Body &root;
	app::Assets &assets;
	std::set<Body *> bodies;
	std::set<Planet *> planets;
	std::set<Sun *> suns;
	double time;

};

}
}

#endif
