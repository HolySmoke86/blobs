#include "Simulation.hpp"

#include "Body.hpp"


namespace blobs {
namespace world {

Simulation::Simulation(Body &r)
: root(r)
, all_bodies()
, time(0.0) {
	AddBody(r);
}

Simulation::~Simulation() {
}


void Simulation::AddBody(Body &b) {
	b.SetSimulation(*this);
	all_bodies.push_back(&b);
}

void Simulation::Tick() {
	constexpr double dt = 0.01666666666666666666666666666666;
	time += dt;
	for (auto body : all_bodies) {
		body->Rotation(body->Rotation() + dt * body->AngularMomentum() / body->Inertia());
	}
}

}
}
