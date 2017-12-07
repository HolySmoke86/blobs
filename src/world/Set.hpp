#ifndef BLOBS_WORLD_SET_HPP_
#define BLOBS_WORLD_SET_HPP_

#include <map>
#include <stdexcept>
#include <string>
#include <vector>

namespace blobs {
namespace world {

template<class Type>
class Set {

public:
	int Add(const Type &t) {
		int id = types.size();
		if (!names.emplace(t.name, id).second) {
			throw std::runtime_error("duplicate type name " + t.name);
		}
		types.emplace_back(t);
		types.back().id = id;
		return id;
	}
	bool Has(int id) const noexcept { return id < types.size(); }
	bool Has(const std::string &name) const noexcept { return names.find(name) != names.end(); }

	typename std::vector<Type>::size_type Size() const noexcept { return types.size(); }

	Type &operator [](int id) noexcept { return types[id]; }
	const Type &operator [](int id) const noexcept { return types[id]; }

	Type &operator [](const std::string &name) {
		auto entry = names.find(name);
		if (entry != names.end()) {
			return types[entry->second];
		} else {
			throw std::runtime_error("unknown type " + name);
		}
	}
	const Type &operator [](const std::string &name) const {
		auto entry = names.find(name);
		if (entry != names.end()) {
			return types[entry->second];
		} else {
			throw std::runtime_error("unknown type " + name);
		}
	}

private:
	std::vector<Type> types;
	std::map<std::string, int> names;

};

}
}

#endif
