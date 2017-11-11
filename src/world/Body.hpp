#ifndef BLOBS_WORLD_BODY_HPP_
#define BLOBS_WORLD_BODY_HPP_

#include "Orbit.hpp"
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

	const std::vector<Body *> &Children() const noexcept { return children; }

	double Mass() const noexcept { return mass; }
	void Mass(double m) noexcept { mass = m; }

	double Radius() const noexcept { return radius; }
	void Radius(double r) noexcept { radius = r; }

	Orbit &GetOrbit() noexcept { return orbit; }
	const Orbit &GetOrbit() const noexcept { return orbit; }

	const glm::dvec2 &SurfaceTilt() const noexcept { return surface_tilt; }
	void SurfaceTilt(const glm::dvec2 &t) noexcept { surface_tilt = t; }

	const glm::dvec2 &AxialTilt() const noexcept { return axis_tilt; }
	void AxialTilt(const glm::dvec2 &t) noexcept { axis_tilt = t; }

	double Rotation() const noexcept { return rotation; }
	void Rotation(double r) noexcept { rotation = r; }

	double AngularMomentum() const noexcept { return angular; }
	void AngularMomentum(double m) noexcept { angular = m; }

	double Inertia() const noexcept;

	double GravitationalParameter() const noexcept;
	double OrbitalPeriod() const noexcept;
	double RotationalPeriod() const noexcept;

	glm::dmat4 LocalTransform() const noexcept;
	glm::dmat4 InverseTransform() const noexcept;

	glm::dmat4 ToParent() const noexcept;
	glm::dmat4 FromParent() const noexcept;

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
	Orbit orbit;
	glm::dvec2 surface_tilt;
	glm::dvec2 axis_tilt;
	double rotation;
	double angular;

};

}
}

#endif
