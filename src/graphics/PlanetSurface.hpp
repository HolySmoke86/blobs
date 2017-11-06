#ifndef BLOBS_GRAPHICS_PLANETSURFACE_HPP_
#define BLOBS_GRAPHICS_PLANETSURFACE_HPP_

#include "Program.hpp"


namespace blobs {
namespace graphics {

class ArrayTexture;

class PlanetSurface {

public:
	PlanetSurface();
	~PlanetSurface();

	PlanetSurface(const PlanetSurface &) = delete;
	PlanetSurface &operator =(const PlanetSurface &) = delete;

	PlanetSurface(PlanetSurface &&) = delete;
	PlanetSurface &operator =(PlanetSurface &&) = delete;

public:
	void Activate() noexcept;

	void SetMVP(const glm::mat4 &m, const glm::mat4 &v, const glm::mat4 &p) noexcept;
	void SetNormal(const glm::vec3 &) noexcept;
	void SetTexture(ArrayTexture &) noexcept;

private:
	Program prog;

	GLuint m_handle;
	GLuint mv_handle;
	GLuint mvp_handle;
	GLuint sampler_handle;
	GLuint normal_handle;

};

}
}

#endif
