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
class Planet;

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

	void RequireBreathing(int r) noexcept { breathes = r; }
	int Breathes() const noexcept { return breathes; }
	bool MustBreathe() const noexcept { return breathes > -1; }

	void RequireDrinking(int r) noexcept { drinks = r; }
	int Drinks() const noexcept { return drinks; }
	bool MustDrink() const noexcept { return drinks > -1; }

	void RequireEating(int r) noexcept { eats = r; }
	int Eats() const noexcept { return eats; }
	bool MustEat() const noexcept { return eats > -1; }

	glm::dmat4 LocalTransform() noexcept;

	void BuildVAO();
	void Draw(app::Assets &, graphics::Viewport &);

private:
	Body *body;
	int surface;
	glm::dvec3 position;

	int breathes;
	int drinks;
	int eats;

	struct Attributes {
		glm::vec3 position;
		glm::vec3 normal;
		glm::vec3 texture;
	};
	graphics::SimpleVAO<Attributes, unsigned short> vao;

};

/// put creature on planet and configure it to (hopefully) survive
void Spawn(Creature &, Planet &, app::Assets &);

}
}

#endif
