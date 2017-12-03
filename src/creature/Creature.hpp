#ifndef BLOBS_CREATURE_CREATURE_HPP_
#define BLOBS_CREATURE_CREATURE_HPP_

#include "Composition.hpp"
#include "Genome.hpp"
#include "Goal.hpp"
#include "Memory.hpp"
#include "Situation.hpp"
#include "Steering.hpp"
#include "../graphics/SimpleVAO.hpp"
#include "../math/geometry.hpp"
#include "../math/glm.hpp"

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
	using Callback = std::function<void(Creature &)>;

	struct Stat {
		// [0,1], zero being good, one bad
		double value = 0.0;
		// static gain per second
		double gain = 0.0;
		// adjust value by delta
		void Add(double delta) noexcept {
			value = glm::clamp(value + delta, 0.0, 1.0);
		}
		bool Empty() const noexcept { return value < 0.000001; }
		bool Good() const noexcept { return value < 0.25; }
		bool Okay() const noexcept { return value < 0.5; }
		bool Bad() const noexcept { return !Okay(); }
		bool Critical() const noexcept { return value > 0.75; }
		bool Full() const noexcept { return value > 0.999999; }
	};
	struct Stats {
		Stat stat[7];
		Stat &Damage() noexcept { return stat[0]; }
		const Stat &Damage() const noexcept { return stat[0]; }
		Stat &Breath() noexcept { return stat[1]; }
		const Stat &Breath() const noexcept { return stat[1]; }
		Stat &Thirst() noexcept { return stat[2]; }
		const Stat &Thirst() const noexcept { return stat[2]; }
		Stat &Hunger() noexcept { return stat[3]; }
		const Stat &Hunger() const noexcept { return stat[3]; }
		Stat &Exhaustion() noexcept { return stat[4]; }
		const Stat &Exhaustion() const noexcept { return stat[4]; }
		Stat &Fatigue() noexcept { return stat[5]; }
		const Stat &Fatigue() const noexcept { return stat[5]; }
		Stat &Boredom() noexcept { return stat[6]; }
		const Stat &Boredom() const noexcept { return stat[6]; }
	};

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

	Genome &GetGenome() noexcept { return genome; }
	const Genome &GetGenome() const noexcept { return genome; }

	Genome::Properties<double> &GetProperties() noexcept { return properties; }
	const Genome::Properties<double> &GetProperties() const noexcept { return properties; }

	void AddMass(int res, double amount);
	const Composition &GetComposition() const noexcept { return composition; }

	void BaseColor(const glm::dvec3 &c) noexcept { base_color = c; }
	const glm::dvec3 &BaseColor() const noexcept { return base_color; }

	void HighlightColor(const glm::dvec3 &c) noexcept;
	glm::dvec4 HighlightColor() const noexcept { return highlight_color; }

	void Mass(double m) noexcept { mass = m; }
	double Mass() const noexcept { return mass; }
	void Ingest(int res, double amount) noexcept;

	void DoWork(double amount) noexcept;

	void Size(double s) noexcept { size = s; }
	double Size() const noexcept { return size; }

	double Born() const noexcept { return birth; }
	double Age() const noexcept;
	/// age-depended multiplier, peak being the maximum in lifetime [0,1]
	double AgeFactor(double peak) const noexcept;

	double EnergyEfficiency() const noexcept;
	double ExhaustionFactor() const noexcept;
	double FatigueFactor() const noexcept;

	// stats with effects applied
	double Strength() const noexcept;
	double Stamina() const noexcept;
	double Dexerty() const noexcept;
	double Intelligence() const noexcept;
	double Lifetime() const noexcept;
	double Fertility() const noexcept;
	double Mutability() const noexcept;
	double Adaptability() const noexcept;
	double OffspringMass() const noexcept;

	/// chance of giving birth per tick
	double OffspringChance() const noexcept;
	/// chance of arbitrary genetic mutation per tick
	double MutateChance() const noexcept;
	/// chance of environmental genetic mutation per tick
	double AdaptChance() const noexcept;

	void Hurt(double d) noexcept;
	void Die() noexcept;
	void OnDeath(Callback cb) noexcept { on_death = cb; }
	void Remove() noexcept;
	bool Removable() const noexcept { return removable; }
	void Removed() noexcept;

	void AddParent(Creature &);
	const std::vector<Creature *> &Parents() const noexcept { return parents; }

	Stats &GetStats() noexcept { return stats; }
	const Stats &GetStats() const noexcept { return stats; }

	Memory &GetMemory() noexcept { return memory; }
	const Memory &GetMemory() const noexcept { return memory; }

	/// constantly active goal. every creature in the simulation is required to have one
	void SetBackgroundTask(std::unique_ptr<Goal> &&g) { bg_task = std::move(g); }
	Goal &BackgroundTask() { return *bg_task; }

	void AddGoal(std::unique_ptr<Goal> &&);
	const std::vector<std::unique_ptr<Goal>> &Goals() const noexcept { return goals; }

	void Tick(double dt);

	Situation &GetSituation() noexcept { return situation; }
	const Situation &GetSituation() const noexcept { return situation; }

	Steering &GetSteering() noexcept { return steering; }
	const Steering &GetSteering() const noexcept { return steering; }

	math::AABB CollisionBox() const noexcept;
	glm::dmat4 CollisionTransform() const noexcept;

	glm::dmat4 LocalTransform() noexcept;

	void BuildVAO();
	void KillVAO();
	void Draw(graphics::Viewport &);

private:
	void TickState(double dt);
	void TickStats(double dt);
	void TickBrain(double dt);
	Situation::Derivative Step(const Situation::Derivative &ds, double dt) const noexcept;

private:
	world::Simulation &sim;
	std::string name;

	Genome genome;
	Genome::Properties<double> properties;
	Composition composition;

	glm::dvec3 base_color;
	glm::dvec4 highlight_color;

	double mass;
	double size;

	double birth;
	double death;
	Callback on_death;
	bool removable;

	std::vector<Creature *> parents;

	Stats stats;
	Memory memory;

	std::unique_ptr<Goal> bg_task;
	std::vector<std::unique_ptr<Goal>> goals;

	Situation situation;
	Steering steering;

	struct Attributes {
		glm::vec3 position;
		glm::vec3 normal;
		glm::vec3 texture;
	};
	std::unique_ptr<graphics::SimpleVAO<Attributes, unsigned short>> vao;

};

/// put creature on planet and configure it to (hopefully) survive
void Spawn(Creature &, world::Planet &);

/// split the creature into two
void Split(Creature &);

}
}

#endif
