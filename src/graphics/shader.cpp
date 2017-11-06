#include "PlanetSurface.hpp"
#include "Program.hpp"
#include "Shader.hpp"

#include "ArrayTexture.hpp"
#include "../app/init.hpp"

#include <algorithm>
#include <iostream>
#include <memory>
#include <ostream>
#include <stdexcept>
#include <string>
#include <glm/gtc/type_ptr.hpp>


namespace {

void gl_error(std::string msg) {
	const GLubyte *errBegin = gluErrorString(glGetError());
	if (errBegin && *errBegin != '\0') {
		const GLubyte *errEnd = errBegin;
		while (*errEnd != '\0') {
			++errEnd;
		}
		msg += ": ";
		msg.append(errBegin, errEnd);
	}
	throw std::runtime_error(msg);
}

}

namespace blobs {
namespace graphics {

Shader::Shader(GLenum type)
: handle(glCreateShader(type)) {
	if (handle == 0) {
		gl_error("glCreateShader");
	}
}

Shader::~Shader() {
	if (handle != 0) {
		glDeleteShader(handle);
	}
}

Shader::Shader(Shader &&other) noexcept
: handle(other.handle) {
	other.handle = 0;
}

Shader &Shader::operator =(Shader &&other) noexcept {
	std::swap(handle, other.handle);
	return *this;
}


void Shader::Source(const GLchar *src) noexcept {
	const GLchar* src_arr[] = { src };
	glShaderSource(handle, 1, src_arr, nullptr);
}

void Shader::Compile() noexcept {
	glCompileShader(handle);
}

bool Shader::Compiled() const noexcept {
	GLint compiled = GL_FALSE;
	glGetShaderiv(handle, GL_COMPILE_STATUS, &compiled);
	return compiled == GL_TRUE;
}

void Shader::Log(std::ostream &out) const {
	int log_len = 0, max_len = 0;
	glGetShaderiv(handle, GL_INFO_LOG_LENGTH, &max_len);
	std::unique_ptr<char[]> log(new char[max_len]);
	glGetShaderInfoLog(handle, max_len, &log_len, log.get());
	out.write(log.get(), log_len);
}


void Shader::AttachToProgram(GLuint id) const noexcept {
	glAttachShader(id, handle);
}


Program::Program()
: handle(glCreateProgram()) {
	if (handle == 0) {
		gl_error("glCreateProgram");
	}
}

Program::~Program() {
	if (handle != 0) {
		glDeleteProgram(handle);
	}
}


const Shader &Program::LoadShader(GLenum type, const GLchar *src) {
	shaders.emplace_back(type);
	Shader &shader = shaders.back();
	shader.Source(src);
	shader.Compile();
	if (!shader.Compiled()) {
		shader.Log(std::cerr);
		throw std::runtime_error("compile shader");
	}
	Attach(shader);
	return shader;
}

void Program::Attach(Shader &shader) noexcept {
	shader.AttachToProgram(handle);
}

void Program::Link() noexcept {
	glLinkProgram(handle);
}

bool Program::Linked() const noexcept {
	GLint linked = GL_FALSE;
	glGetProgramiv(handle, GL_LINK_STATUS, &linked);
	return linked == GL_TRUE;
}

void Program::Log(std::ostream &out) const {
	int log_len = 0, max_len = 0;
	glGetProgramiv(handle, GL_INFO_LOG_LENGTH, &max_len);
	std::unique_ptr<char[]> log(new char[max_len]);
	glGetProgramInfoLog(handle, max_len, &log_len, log.get());
	out.write(log.get(), log_len);
}


GLint Program::AttributeLocation(const GLchar *name) const noexcept {
	return glGetAttribLocation(handle, name);
}

GLint Program::UniformLocation(const GLchar *name) const noexcept {
	return glGetUniformLocation(handle, name);
}


void Program::Uniform(GLint loc, GLint val) noexcept {
	glUniform1i(loc, val);
}

void Program::Uniform(GLint loc, float val) noexcept {
	glUniform1f(loc, val);
}

void Program::Uniform(GLint loc, const glm::vec3 &val) noexcept {
	glUniform3fv(loc, 1, glm::value_ptr(val));
}

void Program::Uniform(GLint loc, const glm::vec4 &val) noexcept {
	glUniform4fv(loc, 1, glm::value_ptr(val));
}

void Program::Uniform(GLint loc, const glm::mat4 &val) noexcept {
	glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(val));
}


PlanetSurface::PlanetSurface()
: prog() {
	prog.LoadShader(
		GL_VERTEX_SHADER,
		"#version 330 core\n"

		"layout(location = 0) in vec3 vtx_position;\n"
		"layout(location = 1) in vec3 vtx_tex_uv;\n"

		"uniform mat4 M;\n"
		"uniform mat4 MV;\n"
		"uniform mat4 MVP;\n"

		"out vec3 frag_tex_uv;\n"
		"out vec3 vtx_viewspace;\n"

		"void main() {\n"
			"gl_Position = MVP * vec4(vtx_position, 1);\n"
			"vtx_viewspace = (MV * vec4(vtx_position, 1)).xyz;\n"
			"frag_tex_uv = vtx_tex_uv;\n"
		"}\n"
	);
	prog.LoadShader(
		GL_FRAGMENT_SHADER,
		"#version 330 core\n"

		"in vec3 vtx_viewspace;\n"
		"in vec3 frag_tex_uv;\n"

		"uniform sampler2DArray tex_sampler;\n"
		"uniform vec3 normal;\n"

		"out vec3 color;\n"

		"void main() {\n"
			"vec3 tex_color = texture(tex_sampler, frag_tex_uv).rgb;\n"
			// TODO: lighting
			"color = tex_color;\n"
		"}\n"
	);
	prog.Link();
	if (!prog.Linked()) {
		prog.Log(std::cerr);
		throw std::runtime_error("link program");
	}
	m_handle = prog.UniformLocation("M");
	mv_handle = prog.UniformLocation("MV");
	mvp_handle = prog.UniformLocation("MVP");
	sampler_handle = prog.UniformLocation("tex_sampler");
	normal_handle = prog.UniformLocation("normal");
}

PlanetSurface::~PlanetSurface() {
}

void PlanetSurface::Activate() noexcept {
	prog.Use();
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glDisable(GL_CULL_FACE);
	glDisable(GL_BLEND);
}

void PlanetSurface::SetMVP(const glm::mat4 &m, const glm::mat4 &v, const glm::mat4 &p) noexcept {
	prog.Uniform(m_handle, m);
	glm::mat4 mv(v * m);
	prog.Uniform(mv_handle, mv);
	prog.Uniform(mvp_handle, p * mv);
}

void PlanetSurface::SetNormal(const glm::vec3 &n) noexcept {
	prog.Uniform(normal_handle, n);
}

void PlanetSurface::SetTexture(ArrayTexture &tex) noexcept {
	glActiveTexture(GL_TEXTURE0);
	tex.Bind();
	prog.Uniform(sampler_handle, GLint(0));
}

}
}
