#ifndef BLOBS_GRAPHICS_PLANETSURFACE_HPP_
#define BLOBS_GRAPHICS_PLANETSURFACE_HPP_

#include "Program.hpp"

#include "glm.hpp"


namespace blobs {
namespace graphics {

class ArrayTexture;

class PlanetSurface {

public:
	static constexpr int MAX_LIGHTS = 8;

public:
	PlanetSurface();
	~PlanetSurface();

	PlanetSurface(const PlanetSurface &) = delete;
	PlanetSurface &operator =(const PlanetSurface &) = delete;

	PlanetSurface(PlanetSurface &&) = delete;
	PlanetSurface &operator =(PlanetSurface &&) = delete;

public:
	void Activate() noexcept;

	void SetM(const glm::mat4 &m) noexcept;
	void SetVP(const glm::mat4 &v, const glm::mat4 &p) noexcept;
	void SetMVP(const glm::mat4 &m, const glm::mat4 &v, const glm::mat4 &p) noexcept;
	void SetNormal(const glm::vec3 &) noexcept;
	void SetTexture(ArrayTexture &) noexcept;
	void SetLight(int n, const glm::vec3 &pos, const glm::vec3 &color, float strength) noexcept;
	void SetNumLights(int n) noexcept;

	const glm::mat4 &M() const noexcept { return m; }
	const glm::mat4 &V() const noexcept { return v; }
	const glm::mat4 &P() const noexcept { return p; }
	const glm::mat4 &MV() const noexcept { return mv; }
	const glm::mat4 &MVP() const noexcept { return mvp; }

private:
	Program prog;

	int num_lights;

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

	GLuint num_lights_handle;
	GLuint light_handle[MAX_LIGHTS * 3];

};

}
}

#endif
