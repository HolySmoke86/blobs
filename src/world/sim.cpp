#include "Simulation.hpp"

#include "Body.hpp"
#include "Planet.hpp"
#include "Sun.hpp"


namespace blobs {
namespace world {

Simulation::Simulation(Body &r)
: root(r)
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

void Simulation::Tick() {
	constexpr double dt = 0.01666666666666666666666666666666;
	time += dt;
	for (auto body : bodies) {
		body->Tick(dt);
	}
}

}
}
