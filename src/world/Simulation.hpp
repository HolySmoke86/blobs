#ifndef BLOBS_WORLD_SIMULATION_HPP_
#define BLOBS_WORLD_SIMULATION_HPP_

#include <vector>


namespace blobs {
namespace world {

class Body;

class Simulation {

public:
	explicit Simulation(Body &root);
	~Simulation();

	Simulation(const Simulation &) = delete;
	Simulation &operator =(const Simulation &) = delete;

	Simulation(Simulation &&) = delete;
	Simulation &operator =(Simulation &&) = delete;

public:
	void Tick();

	void AddBody(Body &);

	Body &Root() { return root; }
	const Body &Root() const { return root; }

	double Time() const noexcept { return time; }

private:
	Body &root;
	std::vector<Body *> all_bodies;
	double time;

};

}
}

#endif
