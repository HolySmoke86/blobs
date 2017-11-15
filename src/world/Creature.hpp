#ifndef BLOBS_WORLD_CREATURE_HPP_
#define BLOBS_WORLD_CREATURE_HPP_

#include "../graphics/glm.hpp"
#include "../graphics/SimpleVAO.hpp"


namespace blobs {
namespace app {
	struct Assets;
}
namespace graphics {
	class Viewport;
}
namespace world {

class Body;

class Creature {

public:
	Creature();
	~Creature();

	Creature(const Creature &) = delete;
	Creature &operator =(const Creature &) = delete;

	Creature(Creature &&) = delete;
	Creature &operator =(Creature &&) = delete;

public:
	void SetBody(Body &b) noexcept { body = &b; }
	Body &GetBody() noexcept { return *body; }
	const Body &GetBody() const noexcept { return *body; }

	void Surface(int s) noexcept { surface = s; }
	void Position(const glm::dvec3 &p) noexcept { position = p; }

	glm::dmat4 LocalTransform() noexcept;

	void BuildVAO();
	void Draw(app::Assets &, graphics::Viewport &);

private:
	Body *body;
	int surface;
	glm::dvec3 position;

	struct Attributes {
		glm::vec3 position;
		glm::vec3 normal;
		glm::vec3 texture;
	};
	graphics::SimpleVAO<Attributes, unsigned short> vao;

};

}
}

#endif
