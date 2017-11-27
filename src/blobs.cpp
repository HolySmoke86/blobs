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

namespace {

struct SwitchPanel {
	SwitchPanel(world::Planet &p, app::Application &app, app::MasterState &state)
	: planet(p), app(app), state(state) { }

	void operator ()(creature::Creature &c) {
		if (planet.Creatures().empty()) {
			std::cout << "no more creatures, game over" << std::endl;
			while (app.HasState()) {
				app.PopState();
			}
		} else {
			for (auto a : planet.Creatures()) {
				if (a != &c) {
					state.GetCreaturePanel().Show(*a);
					a->OnDeath([&](creature::Creature &b) { (*this)(b); });
					break;
				}
			}
		}
	}

	world::Planet &planet;
	app::Application &app;
	app::MasterState &state;
};
}

int main(int argc, char *argv[]) {
	app::Init init(true, 8);
	app::Assets assets;

	world::Sun sun;
	sun.Mass(1.0e14);
	sun.Radius(20.0);
	sun.SurfaceTilt(glm::dvec2(PI * 0.25, PI * 0.25));
	sun.AngularMomentum(1.0e13);

	world::Planet planet(25);
	planet.SetParent(sun);
	planet.Mass(1.0e10);
	planet.GetOrbit().SemiMajorAxis(941.7);
	planet.SurfaceTilt(glm::dvec2(PI * 0.25, PI * 0.25));
	planet.AxialTilt(glm::dvec2(PI * 0.127, 0.0));
	planet.AngularMomentum(6.0e10);

	world::Planet moon(3);
	moon.SetParent(planet);
	moon.Mass(1.0e6);
	moon.GetOrbit().SemiMajorAxis(37.0);
	moon.Rotation(PI * 0.25);
	moon.AngularMomentum(1.0e4);

	world::Planet second_planet(9);
	second_planet.SetParent(sun);
	second_planet.Mass(1.0e9);
	second_planet.GetOrbit().SemiMajorAxis(350.0);
	second_planet.SurfaceTilt(glm::dvec2(PI * 0.125, PI * 0.25));
	second_planet.AxialTilt(glm::dvec2(PI * 0.95, 0.0));
	second_planet.AngularMomentum(1.0e8);

	world::Simulation sim(sun, assets);
	sim.AddSun(sun);
	sim.AddPlanet(planet);
	sim.AddPlanet(second_planet);
	sim.AddPlanet(moon);

	world::GenerateEarthlike(assets.data.tile_types, planet);
	planet.Atmosphere(assets.data.resources["air"].id);
	world::GenerateTest(assets.data.tile_types, moon);
	world::GenerateTest(assets.data.tile_types, second_planet);

	std::cout << "length of year: " << planet.OrbitalPeriod() << "s" << std::endl;
	std::cout << "length of moon cycle: " << moon.OrbitalPeriod() << "s" << std::endl;
	std::cout << "length of day: " << planet.RotationalPeriod() << "s" << std::endl;
	std::cout << "days per year: " << (planet.OrbitalPeriod() / planet.RotationalPeriod()) << std::endl;
	std::cout << "moon cycle in days: " << (moon.OrbitalPeriod() / planet.RotationalPeriod()) << std::endl;
	std::cout << "moon cycles per year: " << (planet.OrbitalPeriod() / moon.OrbitalPeriod()) << std::endl;

	auto blob = new creature::Creature(sim);
	blob->Name(assets.name.Sequential());
	Spawn(*blob, planet);
	blob->BuildVAO();

	app::MasterState state(assets, sim);
	state.GetCamera()
		.Reference(planet)
		// sunrise
		//.FirstPerson(0, glm::vec3(0.0f, 0.0f, 0.1f), glm::vec3(1.0f, -0.75f, 0.1f))
		// sunset
		//.FirstPerson(3, glm::vec3(0.0f, 0.0f, 0.1f), glm::vec3(1.0f, -0.75f, 0.1f))
		// from afar
		.MapView(0, glm::vec3(0.0f, 0.0f, 30.0f), 0.0f)
		// from afar, rotating
		//.Orbital(glm::vec3(-60.0f, 0.0f, 0.0f))
	;
	// system view
	//state.GetCamera()
	//	.Reference(sun)
	//	.Orbital(glm::vec3(-500.0f, 500.0f, 500.0f))
	//;
	state.GetCreaturePanel().Show(*blob);

	app::Application app(init.window, init.viewport);
	SwitchPanel swp(planet, app, state);
	blob->OnDeath([&](creature::Creature &c) { swp(c); });
	app.PushState(&state);
	app.Run();

	return 0;
}
