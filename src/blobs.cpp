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

	world::Sun sun;
	sun.Name("Sun");
	sun.Mass(1.0e14);
	sun.Radius(20.0);
	sun.SurfaceTilt(glm::dvec2(PI * 0.25, PI * 0.25));
	sun.AngularMomentum(1.0e13);

	world::Planet planet(25);
	planet.Name("Planet");
	planet.SetParent(sun);
	planet.Mass(1.0e10);
	planet.GetOrbit().SemiMajorAxis(941.7);
	planet.SurfaceTilt(glm::dvec2(PI * 0.25, PI * 0.25));
	planet.AxialTilt(glm::dvec2(PI * 0.127, 0.0));
	planet.AngularMomentum(6.0e10);

	world::Planet moon(3);
	moon.Name("Moon");
	moon.SetParent(planet);
	moon.Mass(1.0e6);
	moon.GetOrbit().SemiMajorAxis(37.0);
	moon.Rotation(PI * 0.25);
	moon.AngularMomentum(1.0e4);

	world::Planet second_planet(9);
	second_planet.Name("Second planet");
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

	auto blob = new creature::Creature(sim);
	blob->Name(assets.name.Sequential());
	Spawn(*blob, planet);
	// decrease chances of ur-blob dying without splitting
	blob->GetProperties().Fertility() = 1.0;
	blob->BuildVAO();

	app::MasterState state(assets, sim);
	state.GetCreaturePanel().Show(*blob);
	state.GetTimePanel().SetBody(planet);

	app::Application app(init.window, init.viewport);
	app.PushState(&state);
	app.Run();

	return 0;
}
