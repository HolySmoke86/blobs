#ifndef BLOBS_WORLD_SIMULATION_HPP_
#define BLOBS_WORLD_SIMULATION_HPP_


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

	Body &Root() { return root; }
	const Body &Root() const { return root; }

private:
	Body &root;

};

}
}

#endif
