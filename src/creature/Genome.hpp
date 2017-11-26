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

	template<class T>
	struct Properties {
		T birth_mass;
		T fertile_mass;
		T max_mass;

		T fertile_age;
		T infertile_age;
		T death_age;

		T fertility;
	};
	Properties<math::Distribution> properties;


	struct Composition {
		// which resource
		int resource;
		// how much contained in the body
		// relative value to determine average density
		math::Distribution mass;
		// how much to circulate
		math::Distribution intake;
		// how important for alive-being
		math::Distribution penalty;
		// how much off the mass may stay in the body
		math::Distribution growth;
	};
	std::vector<Composition> composition;

	void Configure(Creature &) const;

};

}
}

#endif
