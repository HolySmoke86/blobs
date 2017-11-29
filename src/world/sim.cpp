#include "Simulation.hpp"

#include "Body.hpp"
#include "Planet.hpp"
#include "Sun.hpp"


namespace blobs {
namespace world {

Simulation::Simulation(Body &r, app::Assets &assets)
: root(r)
, assets(assets)
, bodies()
, planets()
, suns()
, time(0.0) {
	AddBody(r);
}

Simulation::~Simulation() {
}


void Simulation::AddBody(Body &b) {
	b.SetSimulation(*this);
	bodies.insert(&b);
}

void Simulation::AddPlanet(Planet &p) {
	AddBody(p);
	planets.insert(&p);
}

void Simulation::AddSun(Sun &s) {
	AddBody(s);
	suns.insert(&s);
}

void Simulation::Tick(double dt) {
	time += dt;
	for (auto body : bodies) {
		body->Tick(dt);
	}
}

}
}
