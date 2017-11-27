#ifndef BLOBS_CREATURE_STEERING_HPP_
#define BLOBS_CREATURE_STEERING_HPP_

#include "Situation.hpp"
#include "../math/glm.hpp"


namespace blobs {
namespace creature {

class Steering {

public:
	Steering();
	~Steering();

public:
	void MaxAcceleration(double a) noexcept { max_accel = a; }
	double MaxAcceleration() const noexcept { return max_accel; }

	void MaxSpeed(double s) noexcept { max_speed = s; }
	double MaxSpeed() const noexcept { return max_speed; }

public:
	void Halt() noexcept;
	void GoTo(const glm::dvec3 &) noexcept;

	glm::dvec3 Acceleration(const Situation::State &) const noexcept;

private:
	bool SumForce(glm::dvec3 &out, const glm::dvec3 &in) const noexcept;

private:
	glm::dvec3 seek_target;

	double max_accel = 1.0;
	double max_speed = 1.0;

	bool halting;
	bool seeking;

};

}
}

#endif
