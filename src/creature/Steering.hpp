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
	void MaxAcceleration(double a) noexcept { max_accel = a; }
	double MaxAcceleration() const noexcept { return max_accel; }

	void MaxSpeed(double s) noexcept { max_speed = s; }
	double MaxSpeed() const noexcept { return max_speed; }

public:
	void Separate(double min_distance, double max_lookaround) noexcept;
	void DontSeparate() noexcept;
	void Halt() noexcept;
	void Pass(const glm::dvec3 &) noexcept;
	void GoTo(const glm::dvec3 &) noexcept;

	glm::dvec3 Acceleration(const Situation::State &) const noexcept;

private:
	bool SumForce(glm::dvec3 &out, const glm::dvec3 &in) const noexcept;
	glm::dvec3 TargetVelocity(const Situation::State &, const glm::dvec3 &) const noexcept;

private:
	const Creature &c;
	glm::dvec3 target;

	double max_accel;
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
