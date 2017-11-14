#ifndef BLOBS_WORLD_TILESET_HPP_
#define BLOBS_WORLD_TILESET_HPP_

#include <map>
#include <string>
#include <vector>

namespace blobs {
namespace world {

class TileType;

class TileSet {

public:
	TileSet();
	~TileSet();

	TileSet(const TileSet &) = delete;
	TileSet &operator =(const TileSet &) = delete;

	TileSet(TileSet &&) = delete;
	TileSet &operator =(TileSet &&) = delete;

public:
	int Add(const TileType &);

	TileType &operator [](int id) noexcept { return types[id]; }
	const TileType &operator [](int id) const noexcept { return types[id]; }

	TileType &operator [](const std::string &name);
	const TileType &operator [](const std::string &name) const;

private:
	std::vector<TileType> types;
	std::map<std::string, int> names;

};

}
}

#endif
