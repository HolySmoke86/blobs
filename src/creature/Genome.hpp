#ifndef BLOBS_CREATURE_GENOME_HPP_
#define BLOBS_CREATURE_GENOME_HPP_

#include "../math/Distribution.hpp"

#include <vector>


namespace blobs {
namespace app {
	struct Assets;
}
namespace creature {

class Creature;

struct Genome {

	struct Composition {
		// which resource
		int resource;
		// how much contained in the body
		math::Distribution mass;
		// how much to circulate
		math::Distribution intake;
		// how important for alive-being
		math::Distribution penalty;
	};
	std::vector<Composition> composition;

	void Configure(app::Assets &, Creature &) const;

};

}
}

#endif
