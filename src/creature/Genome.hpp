#ifndef BLOBS_CREATURE_GENOME_HPP_
#define BLOBS_CREATURE_GENOME_HPP_

#include "../math/Distribution.hpp"
#include "../math/GaloisLFSR.hpp"
#include "../math/glm.hpp"

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
		/// every one of these should have at least one
		/// negative impact to prevent super-beings evolving
		T props[8];
		/// power at the cost of higher solid intake
		T &Strength() noexcept { return props[0]; }
		const T &Strength() const noexcept { return props[0]; }
		/// more endurance at the cost of higher liquid intake
		T &Stamina() noexcept { return props[1]; }
		const T &Stamina() const noexcept { return props[1]; }
		/// more speed at the cost of higher fatigue
		T &Dexerty() noexcept { return props[2]; }
		const T &Dexerty() const noexcept { return props[2]; }
		/// higher mental capacity at the cost of boredom
		T &Intelligence() noexcept { return props[3]; }
		const T &Intelligence() const noexcept { return props[3]; }
		/// average lifetime in seconds
		T &Lifetime() noexcept { return props[4]; }
		const T &Lifetime() const noexcept { return props[4]; }
		/// how likely to succeed in reproduction
		T &Fertility() noexcept { return props[5]; }
		const T &Fertility() const noexcept { return props[5]; }
		/// how likely to mutate
		T &Mutability() noexcept { return props[6]; }
		const T &Mutability() const noexcept { return props[6]; }
		/// mass of offspring
		T &OffspringMass() noexcept { return props[7]; }
		const T &OffspringMass() const noexcept { return props[7]; }
	};
	Properties<math::Distribution> properties;

	math::Distribution base_hue;
	math::Distribution base_saturation;
	math::Distribution base_lightness;

	void Configure(Creature &) const;

	static Properties<double> Instantiate(
		const Properties<math::Distribution> &p,
		math::GaloisLFSR &rand
	) noexcept {
		return {
			p.props[0].FakeNormal(rand.SNorm()),
			p.props[1].FakeNormal(rand.SNorm()),
			p.props[2].FakeNormal(rand.SNorm()),
			p.props[3].FakeNormal(rand.SNorm()),
			p.props[4].FakeNormal(rand.SNorm()),
			p.props[5].FakeNormal(rand.SNorm()),
			p.props[6].FakeNormal(rand.SNorm()),
			p.props[7].FakeNormal(rand.SNorm())
		};
	}

};

}
}

#endif
