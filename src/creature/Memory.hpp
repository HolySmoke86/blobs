#ifndef BLOBS_CREATURE_MEMORY_HPP_
#define BLOBS_CREATURE_MEMORY_HPP_

#include "../math/glm.hpp"

#include <map>


namespace blobs {
namespace world {
	class Planet;
}
namespace creature {

class Creature;

class Memory {

public:
	struct Location {
		world::Planet *planet;
		glm::dvec3 position;
	};

public:
	explicit Memory(Creature &);
	~Memory();

public:
	/// remove all memories
	void Erase();

	/// try to remember where stuff was
	/// when true, pos contains an approximation of the
	/// location of the best fitting resource
	bool RememberLocation(const Composition &, glm::dvec3 &pos) const noexcept;

	void Tick(double dt);

private:
	/// track time spent on a tile
	void TrackStay(const Location &, double t);

private:
	Creature &c;

	struct Stay {
		double first_been;
		Location first_loc;
		double last_been;
		Location last_loc;
		double time_spent;
	};
	std::map<int, Stay> known_types;

};

}
}

#endif
