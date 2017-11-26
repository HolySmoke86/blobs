#ifndef BLOBS_WORLD_ORBIT_HPP_
#define BLOBS_WORLD_ORBIT_HPP_

#include "../math/glm.hpp"


namespace blobs {
namespace world {

class Orbit {

public:
	Orbit();
	~Orbit();

public:
	double SemiMajorAxis() const noexcept;
	Orbit &SemiMajorAxis(double) noexcept;

	double Eccentricity() const noexcept;
	Orbit &Eccentricity(double) noexcept;

	double Inclination() const noexcept;
	Orbit &Inclination(double) noexcept;

	double LongitudeAscending() const noexcept;
	Orbit &LongitudeAscending(double) noexcept;

	double ArgumentPeriapsis() const noexcept;
	Orbit &ArgumentPeriapsis(double) noexcept;

	double MeanAnomaly() const noexcept;
	Orbit &MeanAnomaly(double) noexcept;

	/// calculate transformation matrix at position t in the
	/// orbit, measured in degrees from mean anomaly at t=0
	glm::dmat4 Matrix(double t) const noexcept;
	glm::dmat4 InverseMatrix(double t) const noexcept;

private:
	double sma; // semi-major axis
	double ecc; // eccentricity
	double inc; // inclination
	double asc; // longitude of ascending node
	double arg; // argument of periapsis
	double mna; // mean anomaly (at t=0)

};

}
}

#endif
