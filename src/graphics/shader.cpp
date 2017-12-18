#include "AlphaSprite.hpp"
#include "Canvas.hpp"
#include "CreatureSkin.hpp"
#include "PlanetSurface.hpp"
#include "Program.hpp"
#include "Shader.hpp"
#include "SkyBox.hpp"
#include "SunSurface.hpp"

#include "ArrayTexture.hpp"
#include "CubeMap.hpp"
#include "Texture.hpp"
#include "../app/init.hpp"

#include <algorithm>
#include <iostream>
#include <memory>
#include <ostream>
#include <stdexcept>
#include <string>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>


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
		"layout(location = 1) in vec3 vtx_normal;\n"
		"layout(location = 2) in vec3 vtx_tex_uv;\n"
		"layout(location = 3) in float vtx_shiny;\n"
		"layout(location = 4) in float vtx_glossy;\n"
		"layout(location = 5) in float vtx_metallic;\n"

		"uniform mat4 M;\n"
		"uniform mat4 MV;\n"
		"uniform mat4 MVP;\n"

		"out vec3 vtx_viewspace;\n"
		"out vec3 nrm_viewspace;\n"
		"out vec3 frag_tex_uv;\n"
		"out float frag_shiny;\n"
		"out float frag_glossy;\n"
		"out float frag_metallic;\n"

		"void main() {\n"
			"gl_Position = MVP * vec4(vtx_position, 1.0);\n"
			"vtx_viewspace = (MV * vec4(vtx_position, 1.0)).xyz;\n"
			"nrm_viewspace = (MV * vec4(vtx_position, 0.0)).xyz;\n"
			"frag_tex_uv = vtx_tex_uv;\n"
			"frag_shiny = vtx_shiny;\n"
			"frag_glossy = vtx_glossy;\n"
			"frag_metallic = vtx_metallic;\n"
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
		"in vec3 nrm_viewspace;\n"
		"in vec3 frag_tex_uv;\n"
		"in float frag_shiny;\n"
		"in float frag_glossy;\n"
		"in float frag_metallic;\n"

		"uniform sampler2DArray tex_sampler;\n"
		"uniform vec3 ambient;\n"
		"uniform int num_lights;\n"
		"uniform LightSource light[8];\n"

		"out vec3 color;\n"

		"void main() {\n"
			"vec3 normal = normalize(nrm_viewspace);\n"
			"vec3 view_dir = vec3(0.0, 0.0, 1.0);\n"
			"vec3 tex_color = texture(tex_sampler, frag_tex_uv).rgb;\n"
			"vec3 spec_color = mix(vec3(frag_glossy), tex_color, frag_metallic);\n"
			"vec3 total_light = tex_color * ambient;\n"
			"for (int i = 0; i < num_lights; ++i) {\n"
				"vec3 to_light = light[i].position - vtx_viewspace;\n"
				"float distance = length(to_light) + length(vtx_viewspace);\n"
				"vec3 light_dir = normalize(to_light);\n"
				"float attenuation = light[i].strength / (distance * distance);\n"
				"vec3 diffuse = attenuation * max(0.0, dot(normal, light_dir)) * light[i].color * tex_color;\n"
				"vec3 specular = attenuation * light[i].color"
					" * mix(spec_color, vec3(1.0), pow(1.0 - max(0.0, dot(normalize(light_dir + view_dir), view_dir)), 5.0))"
					" * pow(max(0.0, dot(reflect(-light_dir, normal), view_dir)), frag_shiny);\n"
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
	ambient_handle = prog.UniformLocation("ambient");
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

void PlanetSurface::SetV(const glm::mat4 &vv) noexcept {
	v = vv;
	mv = v * m;
	mvp = p * mv;
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

void PlanetSurface::SetTexture(ArrayTexture &tex) noexcept {
	glActiveTexture(GL_TEXTURE0);
	tex.Bind();
	prog.Uniform(sampler_handle, GLint(0));
}

void PlanetSurface::SetAmbient(const glm::vec3 &a) noexcept {
	prog.Uniform(ambient_handle, a);
}

void PlanetSurface::SetLight(int n, const glm::vec3 &pos, const glm::vec3 &color, float strength) noexcept {
	prog.Uniform(light_handle[3 * n + 0], pos);
	prog.Uniform(light_handle[3 * n + 1], color);
	prog.Uniform(light_handle[3 * n + 2], strength);
}

void PlanetSurface::SetNumLights(int n) noexcept {
	prog.Uniform(num_lights_handle, std::min(MAX_LIGHTS, n));
}


SkyBox::SkyBox()
: prog()
, v(1.0f)
, p(1.0f)
, vp(1.0f) {
	prog.LoadShader(
		GL_VERTEX_SHADER,
		"#version 330 core\n"

		"layout(location = 0) in vec3 vtx_position;\n"

		"uniform mat4 VP;\n"

		"out vec3 vtx_viewspace;\n"

		"void main() {\n"
			"gl_Position = VP * vec4(vtx_position, 1.0);\n"
			"gl_Position.z = gl_Position.w;\n"
			"vtx_viewspace = vtx_position;\n"
		"}\n"
	);
	prog.LoadShader(
		GL_FRAGMENT_SHADER,
		"#version 330 core\n"

		"in vec3 vtx_viewspace;\n"

		"uniform samplerCube tex_sampler;\n"

		"out vec3 color;\n"

		"void main() {\n"
			"color = texture(tex_sampler, vtx_viewspace).rgb;\n"
		"}\n"
	);
	prog.Link();
	if (!prog.Linked()) {
		prog.Log(std::cerr);
		throw std::runtime_error("link program");
	}
	vp_handle = prog.UniformLocation("VP");
	sampler_handle = prog.UniformLocation("tex_sampler");

	vao.Bind();
	vao.BindAttributes();
	vao.EnableAttribute(0);
	vao.AttributePointer<glm::vec3>(0, false, 0);
	vao.ReserveAttributes(8, GL_STATIC_DRAW);
	{
		auto attrib = vao.MapAttributes(GL_WRITE_ONLY);
		attrib[0] = glm::vec3(-1.0f, -1.0f, -1.0f);
		attrib[1] = glm::vec3(-1.0f, -1.0f,  1.0f);
		attrib[2] = glm::vec3(-1.0f,  1.0f, -1.0f);
		attrib[3] = glm::vec3(-1.0f,  1.0f,  1.0f);
		attrib[4] = glm::vec3( 1.0f, -1.0f, -1.0f);
		attrib[5] = glm::vec3( 1.0f, -1.0f,  1.0f);
		attrib[6] = glm::vec3( 1.0f,  1.0f, -1.0f);
		attrib[7] = glm::vec3( 1.0f,  1.0f,  1.0f);
	}
	vao.BindElements();
	vao.ReserveElements(14, GL_STATIC_DRAW);
	{
		auto element = vao.MapElements(GL_WRITE_ONLY);
		element[ 0] = 1;
		element[ 1] = 0;
		element[ 2] = 3;
		element[ 3] = 2;
		element[ 4] = 6;
		element[ 5] = 0;
		element[ 6] = 4;
		element[ 7] = 1;
		element[ 8] = 5;
		element[ 9] = 3;
		element[10] = 7;
		element[11] = 6;
		element[12] = 5;
		element[13] = 4;
	}
	vao.Unbind();
}

SkyBox::~SkyBox() {
}

void SkyBox::Activate() noexcept {
	prog.Use();
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glDisable(GL_CULL_FACE);
	glDisable(GL_BLEND);
}

void SkyBox::SetV(const glm::mat4 &vv) noexcept {
	v = vv;
	v[0].w = 0.0f;
	v[1].w = 0.0f;
	v[2].w = 0.0f;
	v[3] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	vp = p * v;
	prog.Uniform(vp_handle, vp);
}

void SkyBox::SetP(const glm::mat4 &pp) noexcept {
	p = pp;
	vp = p * v;
	prog.Uniform(vp_handle, vp);
}

void SkyBox::SetVP(const glm::mat4 &vv, const glm::mat4 &pp) noexcept {
	p = pp;
	SetV(vv);
}

void SkyBox::SetTexture(CubeMap &cm) noexcept {
	glActiveTexture(GL_TEXTURE0);
	cm.Bind();
	prog.Uniform(sampler_handle, GLint(0));
}

void SkyBox::Draw() const noexcept {
	vao.Bind();
	vao.DrawTriangleStrip(14);
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

	// "resolution" of sphere
	constexpr int size = 10;

	vao.Bind();
	vao.BindAttributes();
	vao.EnableAttribute(0);
	vao.AttributePointer<glm::vec3>(0, false, offsetof(Attributes, position));
	vao.ReserveAttributes(4 * 6 * size * size, GL_STATIC_DRAW);
	{
		auto attrib = vao.MapAttributes(GL_WRITE_ONLY);

		constexpr float radius = float(size) * 0.5f;
		int index = 0;
		for (int surface = 0; surface < 3; ++surface) {
			for (int y = 0; y < size; ++y) {
				for (int x = 0; x < size; ++x, ++index) {
					glm::vec3 pos[4];
					pos[0][(surface + 0) % 3] = float(x + 0) - radius;
					pos[0][(surface + 1) % 3] = float(y + 0) - radius;
					pos[0][(surface + 2) % 3] = radius;
					pos[1][(surface + 0) % 3] = float(x + 0) - radius;
					pos[1][(surface + 1) % 3] = float(y + 1) - radius;
					pos[1][(surface + 2) % 3] = radius;
					pos[2][(surface + 0) % 3] = float(x + 1) - radius;
					pos[2][(surface + 1) % 3] = float(y + 0) - radius;
					pos[2][(surface + 2) % 3] = radius;
					pos[3][(surface + 0) % 3] = float(x + 1) - radius;
					pos[3][(surface + 1) % 3] = float(y + 1) - radius;
					pos[3][(surface + 2) % 3] = radius;
					attrib[4 * index + 0].position = glm::normalize(pos[0]);
					attrib[4 * index + 1].position = glm::normalize(pos[1]);
					attrib[4 * index + 2].position = glm::normalize(pos[2]);
					attrib[4 * index + 3].position = glm::normalize(pos[3]);
				}
			}
		}
		for (int surface = 3; surface < 6; ++surface) {
			for (int y = 0; y < size; ++y) {
				for (int x = 0; x < size; ++x, ++index) {
					glm::vec3 pos[4];
					pos[0][(surface + 0) % 3] = float(x + 0) - radius;
					pos[0][(surface + 1) % 3] = float(y + 0) - radius;
					pos[0][(surface + 2) % 3] = radius;
					pos[1][(surface + 0) % 3] = float(x + 0) - radius;
					pos[1][(surface + 1) % 3] = float(y + 1) - radius;
					pos[1][(surface + 2) % 3] = radius;
					pos[2][(surface + 0) % 3] = float(x + 1) - radius;
					pos[2][(surface + 1) % 3] = float(y + 0) - radius;
					pos[2][(surface + 2) % 3] = radius;
					pos[3][(surface + 0) % 3] = float(x + 1) - radius;
					pos[3][(surface + 1) % 3] = float(y + 1) - radius;
					pos[3][(surface + 2) % 3] = radius;
					attrib[4 * index + 0].position = glm::normalize(pos[0]) * -1.0f;
					attrib[4 * index + 1].position = glm::normalize(pos[1]) * -1.0f;
					attrib[4 * index + 2].position = glm::normalize(pos[2]) * -1.0f;
					attrib[4 * index + 3].position = glm::normalize(pos[3]) * -1.0f;
				}
			}
		}
	}
	vao.BindElements();
	vao.ReserveElements(6 * 6 * size * size, GL_STATIC_DRAW);
	{
		auto element = vao.MapElements(GL_WRITE_ONLY);
		int index = 0;
		for (int surface = 0; surface < 3; ++surface) {
			for (int y = 0; y < size; ++y) {
				for (int x = 0; x < size; ++x, ++index) {
					element[6 * index + 0] = 4 * index + 0;
					element[6 * index + 1] = 4 * index + 2;
					element[6 * index + 2] = 4 * index + 1;
					element[6 * index + 3] = 4 * index + 1;
					element[6 * index + 4] = 4 * index + 2;
					element[6 * index + 5] = 4 * index + 3;
				}
			}
		}
		for (int surface = 3; surface < 6; ++surface) {
			for (int y = 0; y < size; ++y) {
				for (int x = 0; x < size; ++x, ++index) {
					element[6 * index + 0] = 4 * index + 0;
					element[6 * index + 1] = 4 * index + 1;
					element[6 * index + 2] = 4 * index + 2;
					element[6 * index + 3] = 4 * index + 2;
					element[6 * index + 4] = 4 * index + 1;
					element[6 * index + 5] = 4 * index + 3;
				}
			}
		}
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

void SunSurface::SetV(const glm::mat4 &vv) noexcept {
	v = vv;
	mv = v * m;
	mvp = p * mv;
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
	constexpr int size = 10;
	vao.Bind();
	vao.DrawTriangles(6 * 6 * size * size);
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

		"uniform vec3 base_color;\n"
		"uniform vec4 highlight_color;\n"
		"uniform sampler2DArray tex_sampler;\n"
		"uniform vec3 ambient;\n"
		"uniform int num_lights;\n"
		"uniform LightSource light[8];\n"

		"out vec3 color;\n"

		"void main() {\n"
			"vec3 view_dir = vec3(0.0, 0.0, 1.0);\n"
			"vec4 tex_color = texture(tex_sampler, frag_tex_uv);\n"
			"vec3 mat_color = mix(base_color, highlight_color.rgb, tex_color.r * tex_color.a * highlight_color.a);\n"
			"vec3 spec_color = vec3(0.5);\n"
			"vec3 total_light = mat_color * ambient;\n"
			"for (int i = 0; i < num_lights; ++i) {\n"
				"vec3 to_light = light[i].position - vtx_viewspace;\n"
				"float distance = length(to_light) + length(vtx_viewspace);\n"
				"vec3 light_dir = normalize(to_light);\n"
				"float attenuation = light[i].strength / (distance * distance);\n"
				"vec3 diffuse = attenuation * max(0.0, dot(normal, light_dir)) * light[i].color * mat_color;\n"
				"vec3 specular = attenuation * light[i].color"
					" * mix(spec_color, vec3(1.0), pow(1.0 - max(0.0, dot(normalize(light_dir + view_dir), view_dir)), 5.0))"
					" * pow(max(0.0, dot(reflect(-light_dir, normal), view_dir)), 5.0);\n"
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
	base_color_handle = prog.UniformLocation("base_color");
	highlight_color_handle = prog.UniformLocation("highlight_color");
	sampler_handle = prog.UniformLocation("tex_sampler");
	ambient_handle = prog.UniformLocation("ambient");
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

void CreatureSkin::SetV(const glm::mat4 &vv) noexcept {
	v = vv;
	mv = v * m;
	mvp = p * mv;
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

void CreatureSkin::SetBaseColor(const glm::vec3 &c) noexcept {
	prog.Uniform(base_color_handle, c);
}

void CreatureSkin::SetHighlightColor(const glm::vec4 &c) noexcept {
	prog.Uniform(highlight_color_handle, c);
}

void CreatureSkin::SetTexture(ArrayTexture &tex) noexcept {
	glActiveTexture(GL_TEXTURE0);
	tex.Bind();
	prog.Uniform(sampler_handle, GLint(0));
}

void CreatureSkin::SetAmbient(const glm::vec3 &a) noexcept {
	prog.Uniform(ambient_handle, a);
}

void CreatureSkin::SetLight(int n, const glm::vec3 &pos, const glm::vec3 &color, float strength) noexcept {
	prog.Uniform(light_handle[3 * n + 0], pos);
	prog.Uniform(light_handle[3 * n + 1], color);
	prog.Uniform(light_handle[3 * n + 2], strength);
}

void CreatureSkin::SetNumLights(int n) noexcept {
	prog.Uniform(num_lights_handle, std::min(MAX_LIGHTS, n));
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


Canvas::Canvas()
: prog()
, vao() {
	prog.LoadShader(
		GL_VERTEX_SHADER,
		"#version 330 core\n"

		"layout(location = 0) in vec2 vtx_position;\n"

		"uniform mat4 P;\n"
		"uniform float z;\n"

		"void main() {\n"
			// disamond rule adjust
			//"vec3 position = vtx_position + vec3(0.5, 0.5, 0.0);\n"
			"gl_Position = P * vec4(vtx_position, z, 1);\n"
		"}\n"
	);
	prog.LoadShader(
		GL_FRAGMENT_SHADER,
		"#version 330 core\n"

		"uniform vec4 c;\n"

		"out vec4 color;\n"

		"void main() {\n"
			"color = c;\n"
		"}\n"
	);
	prog.Link();
	if (!prog.Linked()) {
		prog.Log(std::cerr);
		throw std::runtime_error("link program");
	}
	p_handle = prog.UniformLocation("P");
	z_handle = prog.UniformLocation("z");
	c_handle = prog.UniformLocation("c");

	vao.Bind();
	vao.BindAttributes();
	vao.EnableAttribute(0);
	vao.AttributePointer<glm::vec2>(0, false, offsetof(Attributes, position));
	vao.ReserveAttributes(255, GL_DYNAMIC_DRAW);
	vao.BindElements();
	vao.ReserveElements(255, GL_DYNAMIC_DRAW);
	vao.Unbind();
}

Canvas::~Canvas() {
}

void Canvas::Resize(float w, float h) noexcept {
	prog.Uniform(p_handle, glm::ortho(0.0f, w, h, 0.0f, 1.0e4f, -1.0e4f));
}

void Canvas::ZIndex(float z) noexcept {
	prog.Uniform(z_handle, -z);
}

void Canvas::SetColor(const glm::vec4 &color) noexcept {
	prog.Uniform(c_handle, color);
}

void Canvas::Activate() noexcept {
	prog.Use();
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void Canvas::DrawLine(const glm::vec2 &p1, const glm::vec2 &p2, float width) {
	glm::vec2 d = glm::normalize(p2 - p1) * (width * 0.5f);
	glm::vec2 n = glm::vec2(d.y, -d.x);
	vao.Bind();
	vao.BindAttributes();
	{
		auto attr = vao.MapAttributes(GL_WRITE_ONLY);
		attr[0].position = p1 - d + n;
		attr[1].position = p1 - d - n;
		attr[2].position = p2 + d + n;
		attr[3].position = p2 + d - n;
	}
	vao.BindElements();
	{
		auto elem = vao.MapElements(GL_WRITE_ONLY);
		elem[0] = 0;
		elem[1] = 1;
		elem[2] = 2;
		elem[3] = 3;
	}
	vao.DrawTriangleStrip(4);
	vao.Unbind();
}

void Canvas::DrawRect(const glm::vec2 &p1, const glm::vec2 &p2, float width) {
	glm::vec2 min(std::min(p1.x, p2.x), std::min(p1.y, p2.y));
	glm::vec2 max(std::max(p1.x, p2.x), std::max(p1.y, p2.y));
	glm::vec2 dg1(min.x, max.y);
	glm::vec2 dg2(max.x, min.y);
	glm::vec2 d(width * 0.5f, width * 0.5f);
	glm::vec2 n(d.y, -d.x);
	vao.Bind();
	vao.BindAttributes();
	{
		auto attr = vao.MapAttributes(GL_WRITE_ONLY);
		attr[0].position = min + d;
		attr[1].position = min - d;
		attr[2].position = dg1 + n;
		attr[3].position = dg1 - n;
		attr[4].position = max - d;
		attr[5].position = max + d;
		attr[6].position = dg2 - n;
		attr[7].position = dg2 + n;
	}
	vao.BindElements();
	{
		auto elem = vao.MapElements(GL_WRITE_ONLY);
		elem[0] = 0;
		elem[1] = 1;
		elem[2] = 2;
		elem[3] = 3;
		elem[4] = 4;
		elem[5] = 5;
		elem[6] = 6;
		elem[7] = 7;
		elem[8] = 0;
		elem[9] = 1;
	}
	vao.DrawTriangleStrip(10);
	vao.Unbind();
}

void Canvas::FillRect(const glm::vec2 &p1, const glm::vec2 &p2) {
	glm::vec2 min(std::min(p1.x, p2.x), std::min(p1.y, p2.y));
	glm::vec2 max(std::max(p1.x, p2.x), std::max(p1.y, p2.y));
	glm::vec2 dg1(min.x, max.y);
	glm::vec2 dg2(max.x, min.y);
	vao.Bind();
	vao.BindAttributes();
	{
		auto attr = vao.MapAttributes(GL_WRITE_ONLY);
		attr[0].position = min;
		attr[1].position = dg1;
		attr[2].position = dg2;
		attr[3].position = max;
	}
	vao.BindElements();
	{
		auto elem = vao.MapElements(GL_WRITE_ONLY);
		elem[0] = 0;
		elem[1] = 1;
		elem[2] = 2;
		elem[3] = 3;
	}
	vao.DrawTriangleStrip(4);
	vao.Unbind();
}

}
}
