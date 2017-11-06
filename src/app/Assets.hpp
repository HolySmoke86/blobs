#ifndef BLOBS_APP_ASSETS_HPP_
#define BLOBS_APP_ASSETS_HPP_

#include "../graphics/ArrayTexture.hpp"
#include "../graphics/PlanetSurface.hpp"


namespace blobs {
namespace app {

struct Assets {

	struct {
		graphics::ArrayTexture tiles;
	} textures;

	struct {
		graphics::PlanetSurface planet_surface;
	} shaders;

	Assets();
	~Assets();

	Assets(const Assets &) = delete;
	Assets &operator =(const Assets &) = delete;

	Assets(Assets &&) = delete;
	Assets &operator =(Assets &&) = delete;

};

}
}

#endif
