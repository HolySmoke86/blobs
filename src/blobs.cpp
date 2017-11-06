#include "app/Application.hpp"
#include "app/Assets.hpp"
#include "app/init.hpp"
#include "app/MasterState.hpp"
#include "world/Planet.hpp"
#include "world/Simulation.hpp"
#include "world/Sun.hpp"

#include <cstdint>


using namespace blobs;

int main(int argc, char *argv[]) {
	app::Init init;
	app::Assets assets;

	world::Sun sun;
	world::Simulation sim(sun);
	world::Planet planet(3);
	world::GenerateTest(planet);
	planet.SetParent(sun);

	app::MasterState state(assets, sim);
	state.SetReference(planet);
	planet.BuildVAOs();

	app::Application app(init.window, init.viewport);
	app.PushState(&state);
	app.Run();

	return 0;
}
