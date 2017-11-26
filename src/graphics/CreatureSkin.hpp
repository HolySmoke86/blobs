#ifndef BLOBS_GRAPHICS_CREATURESKIN_HPP_
#define BLOBS_GRAPHICS_CREATURESKIN_HPP_

#include "Program.hpp"


namespace blobs {
namespace graphics {

class ArrayTexture;

class CreatureSkin {

public:
	static constexpr int MAX_LIGHTS = 8;

public:
	CreatureSkin();
	~CreatureSkin();

	CreatureSkin(const CreatureSkin &) = delete;
	CreatureSkin &operator =(const CreatureSkin &) = delete;

	CreatureSkin(CreatureSkin &&) = delete;
	CreatureSkin &operator =(CreatureSkin &&) = delete;

public:
	void Activate() noexcept;

	void SetM(const glm::mat4 &m) noexcept;
	void SetVP(const glm::mat4 &v, const glm::mat4 &p) noexcept;
	void SetMVP(const glm::mat4 &m, const glm::mat4 &v, const glm::mat4 &p) noexcept;
	void SetTexture(ArrayTexture &) noexcept;
	void SetLight(int n, const glm::vec3 &pos, const glm::vec3 &color, float strength) noexcept;
	void SetNumLights(int n) noexcept;

	const glm::mat4 &M() const noexcept { return m; }
	const glm::mat4 &V() const noexcept { return v; }
	const glm::mat4 &P() const noexcept { return p; }
	const glm::mat4 &MV() const noexcept { return mv; }
	const glm::mat4 &MVP() const noexcept { return mvp; }

private:
	Program prog;

	int num_lights;

	glm::mat4 m;
	glm::mat4 v;
	glm::mat4 p;
	glm::mat4 mv;
	glm::mat4 mvp;

	GLuint m_handle;
	GLuint mv_handle;
	GLuint mvp_handle;
	GLuint sampler_handle;

	GLuint num_lights_handle;
	GLuint light_handle[MAX_LIGHTS * 3];

};

}
}

#endif
