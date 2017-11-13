#ifndef BLOBS_APP_ASSETS_HPP_
#define BLOBS_APP_ASSETS_HPP_

#include "../graphics/ArrayTexture.hpp"
#include "../graphics/PlanetSurface.hpp"
#include "../graphics/SunSurface.hpp"

#include <string>


namespace blobs {
namespace app {

struct Assets {

	std::string path;
	std::string tile_path;

	struct {
		graphics::ArrayTexture tiles;
	} textures;

	struct {
		graphics::PlanetSurface planet_surface;
		graphics::SunSurface sun_surface;
	} shaders;

	Assets();
	~Assets();

	Assets(const Assets &) = delete;
	Assets &operator =(const Assets &) = delete;

	Assets(Assets &&) = delete;
	Assets &operator =(Assets &&) = delete;

	void LoadTileTexture(const std::string &name, graphics::ArrayTexture &, int layer) const;

};

}
}

#endif
