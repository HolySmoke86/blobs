#include "AlphaSprite.hpp"
#include "CreatureSkin.hpp"
#include "PlainColor.hpp"
#include "PlanetSurface.hpp"
#include "Program.hpp"
#include "Shader.hpp"
#include "SunSurface.hpp"

#include "ArrayTexture.hpp"
#include "Texture.hpp"
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

GLint Program::AttributeLocation(const std::string &name) const noexcept {
	return AttributeLocation(name.c_str());
}

GLint Program::UniformLocation(const GLchar *name) const noexcept {
	return glGetUniformLocation(handle, name);
}

GLint Program::UniformLocation(const std::string &name) const noexcept {
	return UniformLocation(name.c_str());
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


constexpr int PlanetSurface::MAX_LIGHTS;

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

		"struct LightSource {\n"
			"vec3 position;\n"
			"vec3 color;\n"
			"float strength;\n"
		"};\n"

		"in vec3 vtx_viewspace;\n"
		"in vec3 frag_tex_uv;\n"

		"uniform sampler2DArray tex_sampler;\n"
		"uniform vec3 normal;\n"
		"uniform int num_lights;\n"
		"uniform LightSource light[8];\n"

		"out vec3 color;\n"

		"void main() {\n"
			"vec3 tex_color = texture(tex_sampler, frag_tex_uv).rgb;\n"
			"vec3 total_light = tex_color * vec3(0.01, 0.01, 0.01);\n"
			"for (int i = 0; i < num_lights; ++i) {\n"
				"vec3 to_light = light[i].position - vtx_viewspace;\n"
				"float distance = length(to_light) + length(vtx_viewspace);\n"
				"vec3 light_dir = normalize(to_light);\n"
				"float attenuation = light[i].strength / (distance * distance);\n"
				"vec3 diffuse = attenuation * max(0.0, dot(normal, light_dir)) * light[i].color * tex_color;\n"
				"vec3 view_dir = vec3(0.0, 0.0, 1.0);\n"
				"vec3 specular = vec3(0.0, 0.0, 0.0);\n"
				"if (dot(normal, light_dir) >= 0.0) {\n"
					"attenuation * light[i].color * pow(max(0.0, dot(reflect(-light_dir, normal), view_dir)), 25.0);\n"
				"}\n"
				"total_light = total_light + diffuse + specular;\n"
			"}\n"
			"color = total_light;\n"
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
	num_lights_handle = prog.UniformLocation("num_lights");
	for (int i = 0; i < MAX_LIGHTS; ++i) {
		light_handle[3 * i + 0]  = prog.UniformLocation("light[" + std::to_string(i) + "].position");
		light_handle[3 * i + 1]  = prog.UniformLocation("light[" + std::to_string(i) + "].color");
		light_handle[3 * i + 2]  = prog.UniformLocation("light[" + std::to_string(i) + "].strength");
	}
}

PlanetSurface::~PlanetSurface() {
}

void PlanetSurface::Activate() noexcept {
	prog.Use();
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_CULL_FACE);
	glDisable(GL_BLEND);
}

void PlanetSurface::SetM(const glm::mat4 &mm) noexcept {
	m = mm;
	mv = v * m;
	mvp = p * mv;
	prog.Uniform(m_handle, m);
	prog.Uniform(mv_handle, mv);
	prog.Uniform(mvp_handle, mvp);
}

void PlanetSurface::SetVP(const glm::mat4 &vv, const glm::mat4 &pp) noexcept {
	v = vv;
	p = pp;
	mv = v * m;
	mvp = p * mv;
	prog.Uniform(mv_handle, mv);
	prog.Uniform(mvp_handle, mvp);
}

void PlanetSurface::SetMVP(const glm::mat4 &mm, const glm::mat4 &vv, const glm::mat4 &pp) noexcept {
	m = mm;
	v = vv;
	p = pp;
	mv = v * m;
	mvp = p * mv;
	prog.Uniform(m_handle, m);
	prog.Uniform(mv_handle, mv);
	prog.Uniform(mvp_handle, mvp);
}

void PlanetSurface::SetNormal(const glm::vec3 &n) noexcept {
	prog.Uniform(normal_handle, n);
}

void PlanetSurface::SetTexture(ArrayTexture &tex) noexcept {
	glActiveTexture(GL_TEXTURE0);
	tex.Bind();
	prog.Uniform(sampler_handle, GLint(0));
}

void PlanetSurface::SetLight(int n, const glm::vec3 &pos, const glm::vec3 &color, float strength) noexcept {
	prog.Uniform(light_handle[3 * n + 0], pos);
	prog.Uniform(light_handle[3 * n + 1], color);
	prog.Uniform(light_handle[3 * n + 2], strength);
}

void PlanetSurface::SetNumLights(int n) noexcept {
	prog.Uniform(num_lights_handle, std::min(MAX_LIGHTS, n));
}


SunSurface::SunSurface()
: prog() {
	prog.LoadShader(
		GL_VERTEX_SHADER,
		"#version 330 core\n"

		"layout(location = 0) in vec3 vtx_position;\n"

		"uniform mat4 M;\n"
		"uniform mat4 MV;\n"
		"uniform mat4 MVP;\n"

		"out vec3 vtx_viewspace;\n"

		"void main() {\n"
			"gl_Position = MVP * vec4(vtx_position, 1);\n"
			"vtx_viewspace = (MV * vec4(vtx_position, 1)).xyz;\n"
		"}\n"
	);
	prog.LoadShader(
		GL_FRAGMENT_SHADER,
		"#version 330 core\n"

		"in vec3 vtx_viewspace;\n"

		"uniform vec3 light_color;\n"
		"uniform float light_strength;\n"

		"out vec3 color;\n"

		"void main() {\n"
			"vec3 to_light = -vtx_viewspace;\n"
			"float distance = length(to_light);\n"
			//"vec3 light_dir = normalize(to_light);\n"
			"float attenuation = light_strength / (distance * distance);\n"
			"color = attenuation * light_color;\n"
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
	light_color_handle = prog.UniformLocation("light_color");
	light_strength_handle = prog.UniformLocation("light_strength");

	vao.Bind();
	vao.BindAttributes();
	vao.EnableAttribute(0);
	vao.AttributePointer<glm::vec3>(0, false, offsetof(Attributes, position));
	vao.ReserveAttributes(8, GL_STATIC_DRAW);
	{
		auto attrib = vao.MapAttributes(GL_WRITE_ONLY);
		attrib[0].position = glm::vec3(-1.0f, -1.0f, -1.0f);
		attrib[1].position = glm::vec3(-1.0f, -1.0f,  1.0f);
		attrib[2].position = glm::vec3(-1.0f,  1.0f, -1.0f);
		attrib[3].position = glm::vec3(-1.0f,  1.0f,  1.0f);
		attrib[4].position = glm::vec3( 1.0f, -1.0f, -1.0f);
		attrib[5].position = glm::vec3( 1.0f, -1.0f,  1.0f);
		attrib[6].position = glm::vec3( 1.0f,  1.0f, -1.0f);
		attrib[7].position = glm::vec3( 1.0f,  1.0f,  1.0f);
	}
	vao.BindElements();
	vao.ReserveElements(36, GL_STATIC_DRAW);
	{
		auto element = vao.MapElements(GL_WRITE_ONLY);
		// -X
		element[ 0] = 0;
		element[ 1] = 1;
		element[ 2] = 2;
		element[ 3] = 2;
		element[ 4] = 1;
		element[ 5] = 3;
		// -Y
		element[ 6] = 0;
		element[ 7] = 4;
		element[ 8] = 1;
		element[ 9] = 1;
		element[10] = 4;
		element[11] = 5;
		// -Z
		element[12] = 0;
		element[13] = 2;
		element[14] = 4;
		element[15] = 4;
		element[16] = 2;
		element[17] = 6;
		// +Z
		element[18] = 1;
		element[19] = 5;
		element[20] = 3;
		element[21] = 3;
		element[22] = 5;
		element[23] = 7;
		// +Y
		element[24] = 3;
		element[25] = 7;
		element[26] = 2;
		element[27] = 2;
		element[28] = 7;
		element[29] = 6;
		// +X
		element[30] = 5;
		element[31] = 4;
		element[32] = 7;
		element[33] = 7;
		element[34] = 4;
		element[35] = 6;
	}
	vao.Unbind();
}

SunSurface::~SunSurface() {
}

void SunSurface::Activate() noexcept {
	prog.Use();
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_CULL_FACE);
	glDisable(GL_BLEND);
}

void SunSurface::SetM(const glm::mat4 &mm) noexcept {
	m = mm;
	mv = v * m;
	mvp = p * mv;
	prog.Uniform(m_handle, m);
	prog.Uniform(mv_handle, mv);
	prog.Uniform(mvp_handle, mvp);
}

void SunSurface::SetVP(const glm::mat4 &vv, const glm::mat4 &pp) noexcept {
	v = vv;
	p = pp;
	mv = v * m;
	mvp = p * mv;
	prog.Uniform(mv_handle, mv);
	prog.Uniform(mvp_handle, mvp);
}

void SunSurface::SetMVP(const glm::mat4 &mm, const glm::mat4 &vv, const glm::mat4 &pp) noexcept {
	m = mm;
	v = vv;
	p = pp;
	mv = v * m;
	mvp = p * mv;
	prog.Uniform(m_handle, m);
	prog.Uniform(mv_handle, mv);
	prog.Uniform(mvp_handle, mvp);
}

void SunSurface::SetLight(const glm::vec3 &color, float strength) noexcept {
	prog.Uniform(light_color_handle, color);
	prog.Uniform(light_strength_handle, strength);
}

void SunSurface::Draw() const noexcept {
	vao.Bind();
	vao.DrawTriangles(36);
}


constexpr int CreatureSkin::MAX_LIGHTS;

CreatureSkin::CreatureSkin()
: prog() {
	prog.LoadShader(
		GL_VERTEX_SHADER,
		"#version 330 core\n"

		"layout(location = 0) in vec3 vtx_position;\n"
		"layout(location = 1) in vec3 vtx_normal;\n"
		"layout(location = 2) in vec3 vtx_tex_uv;\n"

		"uniform mat4 M;\n"
		"uniform mat4 MV;\n"
		"uniform mat4 MVP;\n"

		"out vec3 vtx_viewspace;\n"
		"out vec3 frag_tex_uv;\n"
		"out vec3 normal;\n"

		"void main() {\n"
			"gl_Position = MVP * vec4(vtx_position, 1);\n"
			"vtx_viewspace = (MV * vec4(vtx_position, 1)).xyz;\n"
			"normal = normalize((MV * vec4(vtx_normal, 0)).xyz);\n"
			"frag_tex_uv = vtx_tex_uv;\n"
		"}\n"
	);
	prog.LoadShader(
		GL_FRAGMENT_SHADER,
		"#version 330 core\n"

		"struct LightSource {\n"
			"vec3 position;\n"
			"vec3 color;\n"
			"float strength;\n"
		"};\n"

		"in vec3 vtx_viewspace;\n"
		"in vec3 frag_tex_uv;\n"
		"in vec3 normal;\n"

		"uniform sampler2DArray tex_sampler;\n"
		"uniform int num_lights;\n"
		"uniform LightSource light[8];\n"

		"out vec3 color;\n"

		"void main() {\n"
			"vec3 tex_color = texture(tex_sampler, frag_tex_uv).rgb;\n"
			"vec3 total_light = tex_color * vec3(0.01, 0.01, 0.01);\n"
			"for (int i = 0; i < num_lights; ++i) {\n"
				"vec3 to_light = light[i].position - vtx_viewspace;\n"
				"float distance = length(to_light) + length(vtx_viewspace);\n"
				"vec3 light_dir = normalize(to_light);\n"
				"float attenuation = light[i].strength / (distance * distance);\n"
				"vec3 diffuse = attenuation * max(0.0, dot(normal, light_dir)) * light[i].color * tex_color;\n"
				"vec3 view_dir = vec3(0.0, 0.0, 1.0);\n"
				"vec3 specular = vec3(0.0, 0.0, 0.0);\n"
				"if (dot(normal, light_dir) >= 0.0) {\n"
					"attenuation * light[i].color * pow(max(0.0, dot(reflect(-light_dir, normal), view_dir)), 25.0);\n"
				"}\n"
				"total_light = total_light + diffuse + specular;\n"
			"}\n"
			"color = total_light;\n"
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
	num_lights_handle = prog.UniformLocation("num_lights");
	for (int i = 0; i < MAX_LIGHTS; ++i) {
		light_handle[3 * i + 0]  = prog.UniformLocation("light[" + std::to_string(i) + "].position");
		light_handle[3 * i + 1]  = prog.UniformLocation("light[" + std::to_string(i) + "].color");
		light_handle[3 * i + 2]  = prog.UniformLocation("light[" + std::to_string(i) + "].strength");
	}
}

CreatureSkin::~CreatureSkin() {
}

void CreatureSkin::Activate() noexcept {
	prog.Use();
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_CULL_FACE);
	glDisable(GL_BLEND);
}

void CreatureSkin::SetM(const glm::mat4 &mm) noexcept {
	m = mm;
	mv = v * m;
	mvp = p * mv;
	prog.Uniform(m_handle, m);
	prog.Uniform(mv_handle, mv);
	prog.Uniform(mvp_handle, mvp);
}

void CreatureSkin::SetVP(const glm::mat4 &vv, const glm::mat4 &pp) noexcept {
	v = vv;
	p = pp;
	mv = v * m;
	mvp = p * mv;
	prog.Uniform(mv_handle, mv);
	prog.Uniform(mvp_handle, mvp);
}

void CreatureSkin::SetMVP(const glm::mat4 &mm, const glm::mat4 &vv, const glm::mat4 &pp) noexcept {
	m = mm;
	v = vv;
	p = pp;
	mv = v * m;
	mvp = p * mv;
	prog.Uniform(m_handle, m);
	prog.Uniform(mv_handle, mv);
	prog.Uniform(mvp_handle, mvp);
}

void CreatureSkin::SetTexture(ArrayTexture &tex) noexcept {
	glActiveTexture(GL_TEXTURE0);
	tex.Bind();
	prog.Uniform(sampler_handle, GLint(0));
}

void CreatureSkin::SetLight(int n, const glm::vec3 &pos, const glm::vec3 &color, float strength) noexcept {
	prog.Uniform(light_handle[3 * n + 0], pos);
	prog.Uniform(light_handle[3 * n + 1], color);
	prog.Uniform(light_handle[3 * n + 2], strength);
}

void CreatureSkin::SetNumLights(int n) noexcept {
	prog.Uniform(num_lights_handle, std::min(MAX_LIGHTS, n));
}


PlainColor::PlainColor()
: prog() {
	prog.LoadShader(
		GL_VERTEX_SHADER,
		"#version 330 core\n"

		"layout(location = 0) in vec3 vtx_position;\n"

		"uniform mat4 M;\n"
		"uniform mat4 MV;\n"
		"uniform mat4 MVP;\n"

		"void main() {\n"
			"gl_Position = MVP * vec4(vtx_position, 1);\n"
		"}\n"
	);
	prog.LoadShader(
		GL_FRAGMENT_SHADER,
		"#version 330 core\n"

		"uniform vec4 fg_color;\n"

		"out vec4 color;\n"

		"void main() {\n"
			"color = fg_color;\n"
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
	fg_color_handle = prog.UniformLocation("fg_color");

	vao.Bind();
	vao.BindAttributes();
	vao.EnableAttribute(0);
	vao.AttributePointer<glm::vec3>(0, false, offsetof(Attributes, position));
	vao.ReserveAttributes(4, GL_STATIC_DRAW);
	{
		auto attrib = vao.MapAttributes(GL_WRITE_ONLY);
		attrib[0].position = glm::vec3(-0.5f, -0.5f, 0.0f);
		attrib[1].position = glm::vec3(-0.5f,  0.5f, 0.0f);
		attrib[2].position = glm::vec3( 0.5f, -0.5f, 0.0f);
		attrib[3].position = glm::vec3( 0.5f,  0.5f, 0.0f);
	}
	vao.BindElements();
	vao.ReserveElements(7, GL_STATIC_DRAW);
	{
		auto element = vao.MapElements(GL_WRITE_ONLY);
		element[ 0] = 0;
		element[ 1] = 3;
		element[ 2] = 2;
		element[ 3] = 0;
		element[ 4] = 1;
		element[ 5] = 3;
		element[ 6] = 2;
	}
	vao.Unbind();
}

PlainColor::~PlainColor() {
}

void PlainColor::Activate() noexcept {
	prog.Use();
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void PlainColor::SetM(const glm::mat4 &mm) noexcept {
	m = mm;
	mv = v * m;
	mvp = p * mv;
	prog.Uniform(m_handle, m);
	prog.Uniform(mv_handle, mv);
	prog.Uniform(mvp_handle, mvp);
}

void PlainColor::SetVP(const glm::mat4 &vv, const glm::mat4 &pp) noexcept {
	v = vv;
	p = pp;
	mv = v * m;
	mvp = p * mv;
	prog.Uniform(mv_handle, mv);
	prog.Uniform(mvp_handle, mvp);
}

void PlainColor::SetMVP(const glm::mat4 &mm, const glm::mat4 &vv, const glm::mat4 &pp) noexcept {
	m = mm;
	v = vv;
	p = pp;
	mv = v * m;
	mvp = p * mv;
	prog.Uniform(m_handle, m);
	prog.Uniform(mv_handle, mv);
	prog.Uniform(mvp_handle, mvp);
}

void PlainColor::SetColor(const glm::vec4 &color) noexcept {
	prog.Uniform(fg_color_handle, color);
}

void PlainColor::DrawRect() const noexcept {
	vao.Bind();
	vao.DrawTriangles(6);
}

void PlainColor::OutlineRect() const noexcept {
	vao.Bind();
	vao.DrawLineLoop(4, 3);
}


AlphaSprite::AlphaSprite()
: prog() {
	prog.LoadShader(
		GL_VERTEX_SHADER,
		"#version 330 core\n"

		"layout(location = 0) in vec3 vtx_position;\n"
		"layout(location = 1) in vec2 vtx_texture;\n"

		"uniform mat4 M;\n"
		"uniform mat4 MV;\n"
		"uniform mat4 MVP;\n"

		"out vec2 frag_tex_uv;\n"

		"void main() {\n"
			"gl_Position = MVP * vec4(vtx_position, 1);\n"
			"frag_tex_uv = vtx_texture;\n"
		"}\n"
	);
	prog.LoadShader(
		GL_FRAGMENT_SHADER,
		"#version 330 core\n"

		"in vec2 frag_tex_uv;\n"

		"uniform sampler2D tex_sampler;\n"
		"uniform vec4 fg_color;\n"
		"uniform vec4 bg_color;\n"

		"out vec4 color;\n"

		"void main() {\n"
			"vec4 tex_color = texture(tex_sampler, frag_tex_uv);\n"
			"vec4 factor = mix(bg_color, fg_color, tex_color.a);\n"
			"color = vec4((tex_color * factor).rgb, factor.a);\n"
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
	fg_color_handle = prog.UniformLocation("fg_color");
	bg_color_handle = prog.UniformLocation("bg_color");

	vao.Bind();
	vao.BindAttributes();
	vao.EnableAttribute(0);
	vao.EnableAttribute(1);
	vao.AttributePointer<glm::vec3>(0, false, offsetof(Attributes, position));
	vao.AttributePointer<glm::vec2>(1, false, offsetof(Attributes, texture));
	vao.ReserveAttributes(4, GL_STATIC_DRAW);
	{
		auto attrib = vao.MapAttributes(GL_WRITE_ONLY);
		attrib[0].position = glm::vec3(-0.5f, -0.5f, 0.0f);
		attrib[0].texture = glm::vec2(0.0f, 0.0f);
		attrib[1].position = glm::vec3(-0.5f,  0.5f, 0.0f);
		attrib[1].texture = glm::vec2(0.0f, 1.0f);
		attrib[2].position = glm::vec3( 0.5f, -0.5f, 0.0f);
		attrib[2].texture = glm::vec2(1.0f, 0.0f);
		attrib[3].position = glm::vec3( 0.5f,  0.5f, 0.0f);
		attrib[3].texture = glm::vec2(1.0f, 1.0f);
	}
	vao.BindElements();
	vao.ReserveElements(7, GL_STATIC_DRAW);
	{
		auto element = vao.MapElements(GL_WRITE_ONLY);
		element[ 0] = 0;
		element[ 1] = 1;
		element[ 2] = 2;
		element[ 3] = 3;
	}
	vao.Unbind();
}

AlphaSprite::~AlphaSprite() {
}

void AlphaSprite::Activate() noexcept {
	prog.Use();
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void AlphaSprite::SetM(const glm::mat4 &mm) noexcept {
	m = mm;
	mv = v * m;
	mvp = p * mv;
	prog.Uniform(m_handle, m);
	prog.Uniform(mv_handle, mv);
	prog.Uniform(mvp_handle, mvp);
}

void AlphaSprite::SetVP(const glm::mat4 &vv, const glm::mat4 &pp) noexcept {
	v = vv;
	p = pp;
	mv = v * m;
	mvp = p * mv;
	prog.Uniform(mv_handle, mv);
	prog.Uniform(mvp_handle, mvp);
}

void AlphaSprite::SetMVP(const glm::mat4 &mm, const glm::mat4 &vv, const glm::mat4 &pp) noexcept {
	m = mm;
	v = vv;
	p = pp;
	mv = v * m;
	mvp = p * mv;
	prog.Uniform(m_handle, m);
	prog.Uniform(mv_handle, mv);
	prog.Uniform(mvp_handle, mvp);
}

void AlphaSprite::SetTexture(Texture &tex) noexcept {
	glActiveTexture(GL_TEXTURE0);
	tex.Bind();
	prog.Uniform(sampler_handle, GLint(0));
}

void AlphaSprite::SetFgColor(const glm::vec4 &color) noexcept {
	prog.Uniform(fg_color_handle, color);
}

void AlphaSprite::SetBgColor(const glm::vec4 &color) noexcept {
	prog.Uniform(bg_color_handle, color);
}

void AlphaSprite::DrawRect() const noexcept {
	vao.Bind();
	vao.DrawTriangleStrip(4);
}

}
}
