#ifndef BLOBLS_CREATURE_COMPOSITION_HPP_
#define BLOBLS_CREATURE_COMPOSITION_HPP_

#include <vector>


namespace blobs {
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
	Composition();
	~Composition();

	Composition(const Composition &) = delete;
	Composition &operator =(const Composition &) = delete;

	Composition(Composition &&) = delete;
	Composition &operator =(Composition &&) = delete;

public:
	void Add(int res, double amount);
	bool Has(int res) const noexcept;
	double Get(int res) const noexcept;

public:
	std::vector<Component>::iterator begin() noexcept { return components.begin(); }
	std::vector<Component>::iterator end() noexcept { return components.end(); }
	std::vector<Component>::const_iterator begin() const noexcept { return components.begin(); }
	std::vector<Component>::const_iterator end() const noexcept { return components.end(); }
	std::vector<Component>::const_iterator cbegin() noexcept { return components.cbegin(); }
	std::vector<Component>::const_iterator cend() noexcept { return components.cend(); }

private:
	std::vector<Component> components;

};

}
}

#endif
