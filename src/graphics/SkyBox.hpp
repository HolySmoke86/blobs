#ifndef BLOBS_GRAPHICS_SKYBOX_HPP_
#define BLOBS_GRAPHICS_SKYBOX_HPP_

#include "Program.hpp"
#include "SimpleVAO.hpp"

#include  <cstdint>


namespace blobs {
namespace graphics {

class CubeMap;

class SkyBox {

public:
	SkyBox();
	~SkyBox();

	SkyBox(const SkyBox &) = delete;
	SkyBox &operator =(const SkyBox &) = delete;

	SkyBox(SkyBox &&) = delete;
	SkyBox &operator =(SkyBox &&) = delete;

public:
	void Activate() noexcept;

	void SetV(const glm::mat4 &v) noexcept;
	void SetP(const glm::mat4 &p) noexcept;
	void SetVP(const glm::mat4 &v, const glm::mat4 &p) noexcept;
	void SetTexture(CubeMap &) noexcept;

	const glm::mat4 &V() const noexcept { return v; }
	const glm::mat4 &P() const noexcept { return p; }
	const glm::mat4 &VP() const noexcept { return vp; }

	void Draw() const noexcept;

private:
	Program prog;

	glm::mat4 v;
	glm::mat4 p;
	glm::mat4 vp;

	GLuint vp_handle;
	GLuint sampler_handle;

	SimpleVAO<glm::vec3, std::uint8_t> vao;

};

}
}

#endif
