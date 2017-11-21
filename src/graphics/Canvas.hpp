#ifndef BLOBS_GRAPHICS_CANVAS_HPP_
#define BLOBS_GRAPHICS_CANVAS_HPP_

#include "glm.hpp"
#include "Program.hpp"
#include "SimpleVAO.hpp"

#include <cstdint>


namespace blobs {
namespace graphics {

class Canvas {

public:
	Canvas();
	~Canvas();

	Canvas(const Canvas &) = delete;
	Canvas &operator =(const Canvas &) = delete;

	Canvas(Canvas &&) = delete;
	Canvas &operator =(Canvas &&) = delete;

public:
	void Resize(float w, float h) noexcept;
	void ZIndex(float) noexcept;
	void SetColor(const glm::vec4 &) noexcept;

	void Activate() noexcept;
	void DrawLine(const glm::vec2 &, const glm::vec2 &, float width = 1.0f);
	void DrawRect(const glm::vec2 &, const glm::vec2 &, float width = 1.0f);
	void FillRect(const glm::vec2 &, const glm::vec2 &);

private:
	Program prog;
	GLuint p_handle;
	GLuint z_handle;
	GLuint c_handle;

	struct Attributes {
		glm::vec2 position;
	};
	SimpleVAO<Attributes, std::uint8_t> vao;

};

}
}

#endif
