#ifndef BLOBS_GRAPHICS_VAO_HPP_
#define BLOBS_GRAPHICS_VAO_HPP_

#include "buffer.hpp"
#include "gl_traits.hpp"

#include <GL/glew.h>


namespace blobs {
namespace graphics {

/// Simple vertex array object based on indexed draw calls with attributes
/// interleaved in a single buffer.
template<class Attributes, class Element>
class SimpleVAO {

public:
	SimpleVAO()
	: vao(0)
	, buffers{0} {
		glGenVertexArrays(1, &vao);
		glGenBuffers(2, buffers);
	}
	~SimpleVAO() noexcept {
		glDeleteBuffers(2, buffers);
		glDeleteVertexArrays(1, &vao);
	}

	SimpleVAO(const SimpleVAO &) = delete;
	SimpleVAO &operator =(const SimpleVAO &) = delete;

public:
	void Bind() const noexcept {
		glBindVertexArray(vao);
	}
	void Unbind() const noexcept {
		glBindVertexArray(0);
	}
	void BindAttributes() const noexcept {
		glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
	}
	void BindElements() const noexcept {
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[1]);
	}
	void EnableAttribute(GLuint index) noexcept {
		glEnableVertexAttribArray(index);
	}
	void DisableAttribute(GLuint index) noexcept {
		glDisableVertexAttribArray(index);
	}
	template<class Attribute>
	void AttributePointer(GLuint index, bool normalized, std::size_t offset) noexcept {
		glVertexAttribPointer(
			index,
			gl_traits<Attribute>::size,
			gl_traits<Attribute>::type,
			normalized,
			sizeof(Attributes),
			reinterpret_cast<const void *>(offset)
		);
	}
	void ReserveAttributes(std::size_t size, GLenum usage) noexcept {
		glBufferData(GL_ARRAY_BUFFER, size * sizeof(Attributes), nullptr, usage);
	}
	MappedBuffer<Attributes> MapAttributes(GLenum access) {
		return MappedBuffer<Attributes>(GL_ARRAY_BUFFER, access);
	}
	void ReserveElements(std::size_t size, GLenum usage) noexcept {
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, size * sizeof(Element), nullptr, usage);
	}
	MappedBuffer<Element> MapElements(GLenum access) {
		return MappedBuffer<Element>(GL_ELEMENT_ARRAY_BUFFER, access);
	}
	void DrawTriangles(std::size_t size, std::size_t offset = 0) const noexcept {
		glDrawElements(GL_TRIANGLES, size, gl_traits<Element>::type, ((Element *) nullptr) + offset);
	}

private:
	GLuint vao;
	GLuint buffers[2];

};

}
}

#endif
