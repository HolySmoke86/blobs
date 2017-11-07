#include "Simulation.hpp"

#include "Body.hpp"


namespace blobs {
namespace world {

Simulation::Simulation(Body &r)
: root(r) {
	r.SetSimulation(*this);
}

Simulation::~Simulation() {
}


void Simulation::Tick() {
	time += 0.01666666666666666666666666666666;
}

}
}
