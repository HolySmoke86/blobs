#ifndef BLOBS_APP_ASSETS_HPP_
#define BLOBS_APP_ASSETS_HPP_

#include "../creature/NameGenerator.hpp"
#include "../graphics/AlphaSprite.hpp"
#include "../graphics/ArrayTexture.hpp"
#include "../graphics/Canvas.hpp"
#include "../graphics/CreatureSkin.hpp"
#include "../graphics/CubeMap.hpp"
#include "../graphics/Font.hpp"
#include "../graphics/PlanetSurface.hpp"
#include "../graphics/SkyBox.hpp"
#include "../graphics/SunSurface.hpp"
#include "../math/GaloisLFSR.hpp"
#include "../world/Resource.hpp"
#include "../world/Set.hpp"
#include "../world/TileType.hpp"

#include <string>


namespace blobs {
namespace io {
	class TokenStreamReader;
}
namespace world {
	class Body;
	class Planet;
	class Simulation;
	class Sun;
}
namespace app {

struct Assets {

	std::string path;
	std::string data_path;
	std::string font_path;
	std::string skin_path;
	std::string sky_path;
	std::string tile_path;

	math::GaloisLFSR random;

	creature::NameGenerator name;

	struct {
		world::Set<world::Resource> resources;
		world::Set<world::TileType> tile_types;
	} data;

	struct {
		graphics::Font large;
		graphics::Font medium;
		graphics::Font small;
	} fonts;

	struct {
		graphics::ArrayTexture tiles;
		graphics::ArrayTexture skins;
		graphics::CubeMap sky;
	} textures;

	struct {
		graphics::AlphaSprite alpha_sprite;
		graphics::Canvas canvas;
		graphics::PlanetSurface planet_surface;
		graphics::SunSurface sun_surface;
		graphics::CreatureSkin creature_skin;
		graphics::SkyBox sky_box;
	} shaders;

	Assets();
	~Assets();

	Assets(const Assets &) = delete;
	Assets &operator =(const Assets &) = delete;

	Assets(Assets &&) = delete;
	Assets &operator =(Assets &&) = delete;

	void ReadResources(io::TokenStreamReader &);
	void ReadTileTypes(io::TokenStreamReader &);

	void LoadTileTexture(const std::string &name, graphics::ArrayTexture &, int layer) const;
	void LoadSkinTexture(const std::string &name, graphics::ArrayTexture &, int layer) const;
	void LoadSkyTexture(const std::string &name, graphics::CubeMap &) const;

	void LoadUniverse(const std::string &name, world::Simulation &) const;
	world::Body *ReadBody(io::TokenStreamReader &, world::Simulation &) const;
	void ReadBodyProperty(const std::string &name, io::TokenStreamReader &, world::Body &, world::Simulation &) const;
	void ReadPlanetProperty(const std::string &name, io::TokenStreamReader &, world::Planet &, world::Simulation &) const;
	void ReadSunProperty(const std::string &name, io::TokenStreamReader &, world::Sun &, world::Simulation &) const;

};

}
}

#endif
