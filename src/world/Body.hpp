#ifndef BLOBS_WORLD_BODY_HPP_
#define BLOBS_WORLD_BODY_HPP_

#include "../graphics/glm.hpp"

#include <vector>


namespace blobs {
namespace app {
	class Assets;
}
namespace graphics {
	class Viewport;
}
namespace world {

class Simulation;

class Body {

public:
	Body();
	~Body();

	Body(const Body &) = delete;
	Body &operator =(const Body &) = delete;

	Body(Body &&) = delete;
	Body &operator =(Body &&) = delete;

public:
	bool HasSimulation() const noexcept { return sim; }
	const Simulation &GetSimulation() const noexcept { return *sim; }
	void SetSimulation(Simulation &) noexcept;

	bool HasParent() const { return parent; }
	Body &Parent() { return *parent; }
	const Body &Parent() const { return *parent; }
	void SetParent(Body &);
	void UnsetParent();

	double Mass() const noexcept;
	void Mass(double) noexcept;

	double Radius() const noexcept;
	void Radius(double) noexcept;

	double SemiMajorAxis() const noexcept;
	void SemiMajorAxis(double) noexcept;

	double Eccentricity() const noexcept;
	void Eccentricity(double) noexcept;

	double Inclination() const noexcept;
	void Inclination(double) noexcept;

	double LongitudeAscending() const noexcept;
	void LongitudeAscending(double) noexcept;

	double ArgumentPeriapsis() const noexcept;
	void ArgumentPeriapsis(double) noexcept;

	double MeanAnomaly() const noexcept;
	void MeanAnomaly(double) noexcept;

	double GravitationalParameter() const noexcept;
	double OrbitalPeriod() const noexcept;

	glm::mat4 ToParent() const noexcept;
	glm::mat4 FromParent() const noexcept;

	virtual void Draw(app::Assets &, graphics::Viewport &) { }

private:
	void AddChild(Body &);
	void RemoveChild(Body &);

private:
	Simulation *sim;
	Body *parent;
	std::vector<Body *> children;
	double mass;
	double radius;

	// Orbit
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
