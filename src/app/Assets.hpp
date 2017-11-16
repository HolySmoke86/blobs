#ifndef BLOBS_APP_ASSETS_HPP_
#define BLOBS_APP_ASSETS_HPP_

#include "../graphics/ArrayTexture.hpp"
#include "../graphics/CreatureSkin.hpp"
#include "../graphics/PlanetSurface.hpp"
#include "../graphics/SunSurface.hpp"
#include "../world/Resource.hpp"
#include "../world/Set.hpp"
#include "../world/TileType.hpp"

#include <string>


namespace blobs {
namespace app {

struct Assets {

	std::string path;
	std::string tile_path;
	std::string skin_path;

	struct {
		world::Set<world::Resource> resources;
		world::Set<world::TileType> tiles;
	} data;

	struct {
		graphics::ArrayTexture tiles;
		graphics::ArrayTexture skins;
	} textures;

	struct {
		graphics::PlanetSurface planet_surface;
		graphics::SunSurface sun_surface;
		graphics::CreatureSkin creature_skin;
	} shaders;

	Assets();
	~Assets();

	Assets(const Assets &) = delete;
	Assets &operator =(const Assets &) = delete;

	Assets(Assets &&) = delete;
	Assets &operator =(Assets &&) = delete;

	void LoadTileTexture(const std::string &name, graphics::ArrayTexture &, int layer) const;
	void LoadSkinTexture(const std::string &name, graphics::ArrayTexture &, int layer) const;

};

}
}

#endif
