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
	app::Init init(true, 8);
	app::Assets assets;

	world::Sun sun;
	sun.Mass(1.0e12);
	sun.Radius(10.0);
	sun.SurfaceTilt(glm::dvec2(PI * 0.25, PI * 0.25));
	sun.AngularMomentum(1.0e9);

	world::Planet planet(11);
	world::GenerateTest(planet);
	planet.SetParent(sun);
	planet.Mass(1.0e9);
	planet.GetOrbit().SemiMajorAxis(941.7);
	planet.SurfaceTilt(glm::dvec2(PI * 0.25, PI * 0.25));
	planet.AxialTilt(glm::dvec2(PI * 0.127, 0.0));
	planet.AngularMomentum(1.25e9);

	world::Planet moon(3);
	world::GenerateTest(moon);
	moon.SetParent(planet);
	moon.Mass(1.0e6);
	moon.GetOrbit().SemiMajorAxis(25.0);
	moon.Rotation(PI * 0.25);
	moon.AngularMomentum(1.0e4);

	world::Simulation sim(sun);
	sim.AddSun(sun);
	sim.AddPlanet(planet);
	sim.AddPlanet(moon);

	std::cout << "length of year: " << planet.OrbitalPeriod() << "s" << std::endl;
	std::cout << "length of moon cycle: " << moon.OrbitalPeriod() << "s" << std::endl;
	std::cout << "length of day: " << planet.RotationalPeriod() << "s" << std::endl;
	std::cout << "days per year: " << (planet.OrbitalPeriod() / planet.RotationalPeriod()) << std::endl;
	std::cout << "moon cycle in days: " << (moon.OrbitalPeriod() / planet.RotationalPeriod()) << std::endl;
	std::cout << "moon cycles per year: " << (planet.OrbitalPeriod() / moon.OrbitalPeriod()) << std::endl;

	app::MasterState state(assets, sim);
	state.GetCamera()
		.Reference(planet)
		// sunrise
		.FirstPerson(0, glm::vec3(0.0f, 0.0f, 0.1f), glm::vec3(1.0f, -0.75f, 0.1f))
		// sunset
		//.FirstPerson(3, glm::vec3(0.0f, 0.0f, 0.1f), glm::vec3(1.0f, -0.75f, 0.1f))
		// from afar
		//.MapView(0, glm::vec3(0.0f, 0.0f, 25.0f), 0.0f)
		// system view
		//.Orbital(glm::vec3(50.0f, 2500.0f, 50.0f));
	;
	planet.BuildVAOs();

	app::Application app(init.window, init.viewport);
	app.PushState(&state);
	app.Run();

	return 0;
}
