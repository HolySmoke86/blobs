#ifndef BLOBS_CREATURE_CREATURE_HPP_
#define BLOBS_CREATURE_CREATURE_HPP_

#include "Goal.hpp"
#include "Need.hpp"
#include "Situation.hpp"
#include "Steering.hpp"
#include "../graphics/glm.hpp"
#include "../graphics/SimpleVAO.hpp"

#include <memory>
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
	class Simulation;
}
namespace creature {

class Creature {

public:
	explicit Creature(world::Simulation &);
	~Creature();

	Creature(const Creature &) = delete;
	Creature &operator =(const Creature &) = delete;

	Creature(Creature &&) = delete;
	Creature &operator =(Creature &&) = delete;

public:
	world::Simulation &GetSimulation() noexcept { return sim; }
	const world::Simulation &GetSimulation() const noexcept { return sim; }

	void Name(const std::string &n) noexcept { name = n; }
	const std::string &Name() const noexcept { return name; }

	void Size(double s) noexcept { size = s; }
	double Size() const noexcept { return size; }

	void Health(double h) noexcept { health = h; }
	double Health() const noexcept { return health; }
	void Hurt(double d) noexcept;

	void AddNeed(std::unique_ptr<Need> &&n) { needs.emplace_back(std::move(n)); }
	const std::vector<std::unique_ptr<Need>> &Needs() const noexcept { return needs; }

	void AddGoal(std::unique_ptr<Goal> &&);
	const std::vector<std::unique_ptr<Goal>> &Goals() const noexcept { return goals; }

	void Tick(double dt);

	Situation &GetSituation() noexcept { return situation; }
	const Situation &GetSituation() const noexcept { return situation; }

	Steering &GetSteering() noexcept { return steering; }
	const Steering &GetSteering() const noexcept { return steering; }

	void Velocity(const glm::dvec3 &v) noexcept { vel = v; }
	const glm::dvec3 &Velocity() const noexcept { return vel; }
	bool Moving() const noexcept { return !allzero(vel); }

	glm::dmat4 LocalTransform() noexcept;

	void BuildVAO();
	void Draw(app::Assets &, graphics::Viewport &);

private:
	world::Simulation &sim;
	std::string name;
	double size;
	double health;

	std::vector<std::unique_ptr<Need>> needs;
	std::vector<std::unique_ptr<Goal>> goals;

	Situation situation;
	Steering steering;

	glm::dvec3 vel;

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
