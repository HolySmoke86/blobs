#ifndef BLOBS_GRAPHICS_SHADER_HPP_
#define BLOBS_GRAPHICS_SHADER_HPP_

#include <iosfwd>
#include <GL/glew.h>


namespace blobs {
namespace graphics {

class Shader {

public:
	explicit Shader(GLenum type);
	~Shader();

	Shader(Shader &&) noexcept;
	Shader &operator =(Shader &&) noexcept;

	Shader(const Shader &) = delete;
	Shader &operator =(const Shader &) = delete;

	void Source(const GLchar *src) noexcept;
	void Compile() noexcept;
	bool Compiled() const noexcept;
	void Log(std::ostream &) const;

	void AttachToProgram(GLuint id) const noexcept;

private:
	GLuint handle;

};

}
}

#endif
