#ifndef BLOBS_GRAPHICS_SUNSURFACE_HPP_
#define BLOBS_GRAPHICS_SUNSURFACE_HPP_

#include "Program.hpp"
#include "SimpleVAO.hpp"

#include "glm.hpp"

#include <cstdint>


namespace blobs {
namespace graphics {

class ArrayTexture;

class SunSurface {

public:
	SunSurface();
	~SunSurface();

	SunSurface(const SunSurface &) = delete;
	SunSurface &operator =(const SunSurface &) = delete;

	SunSurface(SunSurface &&) = delete;
	SunSurface &operator =(SunSurface &&) = delete;

public:
	void Activate() noexcept;

	void SetM(const glm::mat4 &m) noexcept;
	void SetVP(const glm::mat4 &v, const glm::mat4 &p) noexcept;
	void SetMVP(const glm::mat4 &m, const glm::mat4 &v, const glm::mat4 &p) noexcept;
	void SetLight(const glm::vec3 &color, float strength) noexcept;

	const glm::mat4 &M() const noexcept { return m; }
	const glm::mat4 &V() const noexcept { return v; }
	const glm::mat4 &P() const noexcept { return p; }
	const glm::mat4 &MV() const noexcept { return mv; }
	const glm::mat4 &MVP() const noexcept { return mvp; }

	void Draw() const noexcept;

private:
	struct Attributes {
		glm::vec3 position;
	};
	SimpleVAO<Attributes, std::uint8_t> vao;
	Program prog;

	glm::mat4 m;
	glm::mat4 v;
	glm::mat4 p;
	glm::mat4 mv;
	glm::mat4 mvp;

	GLuint m_handle;
	GLuint mv_handle;
	GLuint mvp_handle;

	GLuint light_color_handle;
	GLuint light_strength_handle;

};

}
}

#endif
