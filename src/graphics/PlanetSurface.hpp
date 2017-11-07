#ifndef BLOBS_GRAPHICS_PLANETSURFACE_HPP_
#define BLOBS_GRAPHICS_PLANETSURFACE_HPP_

#include "Program.hpp"

#include "glm.hpp"


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
	void SetLight(const glm::vec3 &pos, const glm::vec3 &color, float strength) noexcept;

	const glm::mat4 &M() const noexcept { return m; }
	const glm::mat4 &V() const noexcept { return v; }
	const glm::mat4 &P() const noexcept { return p; }
	const glm::mat4 &MV() const noexcept { return mv; }
	const glm::mat4 &MVP() const noexcept { return mvp; }

private:
	Program prog;

	glm::mat4 m;
	glm::mat4 v;
	glm::mat4 p;
	glm::mat4 mv;
	glm::mat4 mvp;

	GLuint m_handle;
	GLuint mv_handle;
	GLuint mvp_handle;
	GLuint sampler_handle;
	GLuint normal_handle;

	GLuint light_position_handle;
	GLuint light_color_handle;
	GLuint light_strength_handle;

};

}
}

#endif
