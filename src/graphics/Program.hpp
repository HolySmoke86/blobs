#ifndef BLOBS_GRAPHICS_PROGRAM_HPP_
#define BLOBS_GRAPHICS_PROGRAM_HPP_

#include "../math/glm.hpp"

#include <iosfwd>
#include <list>
#include <string>
#include <GL/glew.h>


namespace blobs {
namespace graphics {

class Shader;

class Program {

public:
	Program();
	~Program();

	Program(const Program &) = delete;
	Program &operator =(const Program &) = delete;

	const Shader &LoadShader(GLenum type, const GLchar *src);
	void Attach(Shader &) noexcept;
	void Link() noexcept;
	bool Linked() const noexcept;
	void Log(std::ostream &) const;

	GLint AttributeLocation(const GLchar *name) const noexcept;
	GLint AttributeLocation(const std::string &name) const noexcept;
	GLint UniformLocation(const GLchar *name) const noexcept;
	GLint UniformLocation(const std::string &name) const noexcept;

	void Uniform(GLint, GLint) noexcept;
	void Uniform(GLint, float) noexcept;
	void Uniform(GLint, const glm::vec3 &) noexcept;
	void Uniform(GLint, const glm::vec4 &) noexcept;
	void Uniform(GLint, const glm::mat4 &) noexcept;

	void Use() const noexcept { glUseProgram(handle); }

private:
	GLuint handle;
	std::list<Shader> shaders;

};

}
}

#endif
