#ifndef BLOBS_WORLD_SIMULATION_HPP_
#define BLOBS_WORLD_SIMULATION_HPP_

#include "Record.hpp"
#include "Set.hpp"
#include "../app/Assets.hpp"

#include <set>
#include <vector>


namespace blobs {
namespace creature {
	class Creature;
}
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

	Body &Root() noexcept { return root; }
	const Body &Root() const noexcept { return root; }

	app::Assets &Assets() noexcept { return assets; }
	const app::Assets &Assets() const noexcept { return assets; }
	const Set<Resource> &Resources() const noexcept { return assets.data.resources; }
	const Set<TileType> &TileTypes() const noexcept { return assets.data.tile_types; }

	void AddBody(Body &);
	void AddPlanet(Planet &);
	void AddSun(Sun &);

	const std::set<Body *> &Bodies() const noexcept { return bodies; }
	const std::set<Planet *> &Planets() const noexcept { return planets; }
	const std::set<Sun *> &Suns() const noexcept { return suns; }

	void SetAlive(creature::Creature *);
	std::vector<creature::Creature *> &LiveCreatures() noexcept { return alive; }
	const std::vector<creature::Creature *> &LiveCreatures() const noexcept { return alive; }

	void SetDead(creature::Creature *);
	std::vector<creature::Creature *> &DeadCreatures() noexcept { return dead; }
	const std::vector<creature::Creature *> &DeadCreatures() const noexcept { return dead; }

	double Time() const noexcept { return time; }

	const std::vector<Record> &Records() const noexcept { return records; }
	void CheckRecords(creature::Creature &) noexcept;

private:
	Body &root;
	app::Assets &assets;

	std::set<Body *> bodies;
	std::set<Planet *> planets;
	std::set<Sun *> suns;

	std::vector<creature::Creature *> alive;
	std::vector<creature::Creature *> dead;

	double time;
	std::vector<Record> records;

};

}
}

#endif
