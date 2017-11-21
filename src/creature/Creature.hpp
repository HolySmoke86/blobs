#ifndef BLOBS_CREATURE_CREATURE_HPP_
#define BLOBS_CREATURE_CREATURE_HPP_

#include "Need.hpp"
#include "Situation.hpp"
#include "../graphics/glm.hpp"
#include "../graphics/SimpleVAO.hpp"

#include <string>
#include <vector>


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
}
namespace creature {

class Creature {

public:
	Creature();
	~Creature();

	Creature(const Creature &) = delete;
	Creature &operator =(const Creature &) = delete;

	Creature(Creature &&) = delete;
	Creature &operator =(Creature &&) = delete;

public:
	void Name(const std::string &n) noexcept { name = n; }
	const std::string &Name() const noexcept { return name; }

	void Health(double h) noexcept { health = h; }
	double Health() const noexcept { return health; }

	void AddNeed(const Need &n) { needs.push_back(n); }
	const std::vector<Need> &Needs() const noexcept { return needs; }

	void Tick(double dt);

	Situation &GetSituation() noexcept { return situation; }
	const Situation &GetSituation() const noexcept { return situation; }

	glm::dmat4 LocalTransform() noexcept;

	void BuildVAO();
	void Draw(app::Assets &, graphics::Viewport &);

private:
	std::string name;
	double health;
	std::vector<Need> needs;

	Situation situation;

	struct Attributes {
		glm::vec3 position;
		glm::vec3 normal;
		glm::vec3 texture;
	};
	graphics::SimpleVAO<Attributes, unsigned short> vao;

};

/// put creature on planet and configure it to (hopefully) survive
void Spawn(Creature &, world::Planet &, app::Assets &);

}
}

#endif
