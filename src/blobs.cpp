#include "app/Application.hpp"
#include "app/Assets.hpp"
#include "app/init.hpp"
#include "app/MasterState.hpp"
#include "creature/Creature.hpp"
#include "math/const.hpp"
#include "world/Planet.hpp"
#include "world/Set.hpp"
#include "world/Simulation.hpp"
#include "world/Sun.hpp"
#include "world/TileType.hpp"

#include <cstdint>
#include <iostream>

using namespace blobs;

int main(int argc, char *argv[]) {
	app::Init init(true, 8);
	app::Assets assets;

	world::Simulation sim(assets);
	assets.LoadUniverse("universe", sim);

	auto blob = new creature::Creature(sim);
	blob->Name(assets.name.Sequential());
	Spawn(*blob, sim.PlanetByName("Planet"));
	// decrease chances of ur-blob dying without splitting
	blob->GetProperties().Fertility() = 1.0;
	blob->BuildVAO();

	app::MasterState state(assets, sim);
	state.Show(*blob);

	app::Application app(init.window, init.viewport);
	app.PushState(&state);
	app.Run();

	return 0;
}
