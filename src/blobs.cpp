#include "const.hpp"
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
	sun.Mass(1.0e13);
	world::Simulation sim(sun);
	world::Planet planet(3);
	planet.Mass(1.0e7);
	planet.Inclination(PI * 0.25);
	world::GenerateTest(planet);
	planet.SetParent(sun);
	planet.SemiMajorAxis(10.0);

	app::MasterState state(assets, sim);
	state.SetReference(planet);
	planet.BuildVAOs();

	app::Application app(init.window, init.viewport);
	app.PushState(&state);
	app.Run();

	return 0;
}
