#ifndef BLOBS_CREATURE_STEERING_HPP_
#define BLOBS_CREATURE_STEERING_HPP_

#include "Situation.hpp"
#include "../math/glm.hpp"


namespace blobs {
namespace creature {

class Creature;

class Steering {

public:
	explicit Steering(const Creature &);
	~Steering();

public:
	/// factor in [0,1] of how fast you need to get there
	void Haste(double h) noexcept { haste = h; }
	double Haste() const noexcept { return haste; }

	void MaxForce(double f) noexcept { max_force = f; }
	double MaxForce() const noexcept { return max_force; }

	void MaxSpeed(double s) noexcept { max_speed = s; }
	double MaxSpeed() const noexcept { return max_speed; }

public:
	void Off() noexcept;
	void Separate(double min_distance, double max_lookaround) noexcept;
	void DontSeparate() noexcept;
	void ResumeSeparate() noexcept;
	void Halt() noexcept;
	void Pass(const glm::dvec3 &) noexcept;
	void GoTo(const glm::dvec3 &) noexcept;

	glm::dvec3 Force(const Situation::State &) const noexcept;

private:
	glm::dvec3 TargetVelocity(const Situation::State &, const glm::dvec3 &, double acc) const noexcept;

private:
	const Creature &c;
	glm::dvec3 target;

	double haste;
	double max_force;
	double max_speed;
	double min_dist;
	double max_look;

	bool separating;
	bool halting;
	bool seeking;
	bool arriving;

};

}
}

#endif
