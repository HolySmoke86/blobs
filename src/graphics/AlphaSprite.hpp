#ifndef BLOBS_GRAPHICS_ALPHASPRITE_HPP_
#define BLOBS_GRAPHICS_ALPHASPRITE_HPP_

#include "Program.hpp"
#include "SimpleVAO.hpp"

#include "glm.hpp"

#include <cstdint>


namespace blobs {
namespace graphics {

class Texture;

class AlphaSprite {

public:
	AlphaSprite();
	~AlphaSprite();

	AlphaSprite(const AlphaSprite &) = delete;
	AlphaSprite &operator =(const AlphaSprite &) = delete;

	AlphaSprite(AlphaSprite &&) = delete;
	AlphaSprite &operator =(AlphaSprite &&) = delete;

public:
	void Activate() noexcept;

	void SetM(const glm::mat4 &m) noexcept;
	void SetVP(const glm::mat4 &v, const glm::mat4 &p) noexcept;
	void SetMVP(const glm::mat4 &m, const glm::mat4 &v, const glm::mat4 &p) noexcept;
	void SetTexture(Texture &) noexcept;
	void SetFgColor(const glm::vec4 &) noexcept;
	void SetBgColor(const glm::vec4 &) noexcept;

	const glm::mat4 &M() const noexcept { return m; }
	const glm::mat4 &V() const noexcept { return v; }
	const glm::mat4 &P() const noexcept { return p; }
	const glm::mat4 &MV() const noexcept { return mv; }
	const glm::mat4 &MVP() const noexcept { return mvp; }

	void DrawRect() const noexcept;

private:
	struct Attributes {
		glm::vec3 position;
		glm::vec2 texture;
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

	GLuint sampler_handle;
	GLuint fg_color_handle;
	GLuint bg_color_handle;

};

}
}

#endif
