#include "const.hpp"
#include "app/Application.hpp"
#include "app/Assets.hpp"
#include "app/init.hpp"
#include "app/MasterState.hpp"
#include "world/Planet.hpp"
#include "world/Simulation.hpp"
#include "world/Sun.hpp"

#include <cstdint>
#include <iostream>


using namespace blobs;

int main(int argc, char *argv[]) {
	app::Init init;
	app::Assets assets;

	world::Sun sun;
	sun.Mass(1.0e12);
	sun.Radius(1.0);
	sun.SurfaceTilt(glm::dvec2(PI * 0.25, PI * 0.25));
	sun.AngularMomentum(1.0e9);

	world::Planet planet(11);
	world::GenerateTest(planet);
	planet.SetParent(sun);
	planet.Mass(1.0e9);
	planet.GetOrbit().SemiMajorAxis(100.0);
	planet.SurfaceTilt(glm::dvec2(PI * 0.25, PI * 0.25));
	planet.AxialTilt(glm::dvec2(PI * 0.127, 0.0));
	planet.AngularMomentum(3.0e9);

	world::Planet moon(3);
	world::GenerateTest(moon);
	moon.SetParent(planet);
	moon.Mass(1.0e6);
	moon.GetOrbit().SemiMajorAxis(25.0);
	moon.AngularMomentum(1.0e5);

	world::Simulation sim(sun);
	sim.AddBody(planet);
	sim.AddBody(moon);

	std::cout << "length of year: " << planet.OrbitalPeriod() << "s" << std::endl;
	std::cout << "length of moon cycle: " << moon.OrbitalPeriod() << "s" << std::endl;
	std::cout << "length of day: " << planet.RotationalPeriod() << "s" << std::endl;

	app::MasterState state(assets, sim);
	state.SetReference(planet);
	planet.BuildVAOs();

	app::Application app(init.window, init.viewport);
	app.PushState(&state);
	app.Run();

	return 0;
}
