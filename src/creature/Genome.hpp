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
	struct PropertySet {
		/// the age at which to transition to the next phase
		T age;
		/// maximum body mass
		T mass;
		/// fertility factor
		T fertility;
		/// skin highlight pronounciation
		T highlight;
	};
	template<class T>
	struct Properties {
		PropertySet<T> props[6];
		PropertySet<T> &Birth() noexcept { return props[0]; }
		const PropertySet<T> &Birth() const noexcept { return props[0]; }
		PropertySet<T> &Child() noexcept { return props[1]; }
		const PropertySet<T> &Child() const noexcept { return props[1]; }
		PropertySet<T> &Youth() noexcept { return props[2]; }
		const PropertySet<T> &Youth() const noexcept { return props[2]; }
		PropertySet<T> &Adult() noexcept { return props[3]; }
		const PropertySet<T> &Adult() const noexcept { return props[3]; }
		PropertySet<T> &Elder() noexcept { return props[4]; }
		const PropertySet<T> &Elder() const noexcept { return props[4]; }
		PropertySet<T> &Death() noexcept { return props[5]; }
		const PropertySet<T> &Death() const noexcept { return props[5]; }

		/// "typical" properties
		/// every one of these should have at least one
		/// negative impact to prevent super-beings evolving
		/// power at the cost of higher solid intake
		T strength;
		/// more endurance at the cost of higher liquid intake
		T stamina;
		/// more speed at the cost of higher fatigue
		T dexerty;
		/// higher mental capacity at the cost of boredom
		T intelligence;
		/// how likely to mutate
		T mutability;
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

	math::Distribution base_hue;
	math::Distribution base_saturation;
	math::Distribution base_lightness;

	void Configure(Creature &) const;

	static PropertySet<double> Instantiate(
		const PropertySet<math::Distribution> &p,
		math::GaloisLFSR &rand
	) noexcept {
		return {
			p.age.FakeNormal(rand.SNorm()),
			p.mass.FakeNormal(rand.SNorm()),
			p.fertility.FakeNormal(rand.SNorm()),
			glm::clamp(p.highlight.FakeNormal(rand.SNorm()), 0.0, 1.0)
		};
	}

	static Properties<double> Instantiate(
		const Properties<math::Distribution> &p,
		math::GaloisLFSR &rand
	) noexcept {
		return {
			Instantiate(p.props[0], rand),
			Instantiate(p.props[1], rand),
			Instantiate(p.props[2], rand),
			Instantiate(p.props[3], rand),
			Instantiate(p.props[4], rand),
			Instantiate(p.props[5], rand),
			p.strength.FakeNormal(rand.SNorm()),
			p.stamina.FakeNormal(rand.SNorm()),
			p.dexerty.FakeNormal(rand.SNorm()),
			p.intelligence.FakeNormal(rand.SNorm()),
			p.mutability.FakeNormal(rand.SNorm())
		};
	}

};

}
}

#endif
