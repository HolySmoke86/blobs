#ifndef BLOBLS_CREATURE_COMPOSITION_HPP_
#define BLOBLS_CREATURE_COMPOSITION_HPP_

#include "../world/Set.hpp"
#include "../world/Resource.hpp"

#include <vector>


namespace blobs {
namespace world {
	class Resource;
}
namespace creature {

class Composition {

public:
	struct Component {
		int resource;
		double value;
		Component(int r, double v)
		: resource(r), value(v) { }
	};

public:
	explicit Composition(const world::Set<world::Resource> &);
	~Composition();

	Composition(const Composition &) = default;
	Composition &operator =(const Composition &) = default;

	Composition(Composition &&) = default;
	Composition &operator =(Composition &&) = default;

public:
	void Add(int res, double amount);
	bool Has(int res) const noexcept;
	double Get(int res) const noexcept;
	double Proportion(int res) const noexcept;
	double StateProportion(int res) const noexcept;
	double Compatibility(int res) const noexcept;
	double TotalMass() const noexcept { return total_mass; }
	double TotalVolume() const noexcept { return total_volume; }
	double TotalDensity() const noexcept { return total_mass / total_volume; }
	double StateMass(world::Resource::State s) const noexcept { return state_mass[s]; }

public:
	std::vector<Component>::size_type size() const noexcept { return components.size(); }
	std::vector<Component>::iterator begin() noexcept { return components.begin(); }
	std::vector<Component>::iterator end() noexcept { return components.end(); }
	std::vector<Component>::const_iterator begin() const noexcept { return components.begin(); }
	std::vector<Component>::const_iterator end() const noexcept { return components.end(); }
	std::vector<Component>::const_iterator cbegin() noexcept { return components.cbegin(); }
	std::vector<Component>::const_iterator cend() noexcept { return components.cend(); }

private:
	const world::Set<world::Resource> &resources;
	std::vector<Component> components;
	double total_mass;
	double total_volume;
	double state_mass[4];

};

}
}

#endif
