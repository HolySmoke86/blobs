#ifndef BLOBS_WORLD_BODY_HPP_
#define BLOBS_WORLD_BODY_HPP_

#include "Orbit.hpp"
#include "../math/glm.hpp"

#include <vector>


namespace blobs {
namespace app {
	class Assets;
}
namespace creature {
	class Creature;
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

	const glm::dmat4 &LocalTransform() const noexcept { return local; }
	const glm::dmat4 &InverseTransform() const noexcept { return inverse_local; }

	const glm::dmat4 &ToParent() const noexcept { return inverse_orbital; }
	const glm::dmat4 &FromParent() const noexcept { return orbital; }

	glm::dmat4 ToUniverse() const noexcept;
	glm::dmat4 FromUniverse() const noexcept;

	virtual void Draw(app::Assets &, graphics::Viewport &) { }

	void Tick(double dt);
	void Cache() noexcept;
	void CheckCollision() noexcept;

	// body takes over ownership of given pointer
	void AddCreature(creature::Creature *);
	void RemoveCreature(creature::Creature *);
	std::vector<creature::Creature *> &Creatures() noexcept { return creatures; }
	const std::vector<creature::Creature *> &Creatures() const noexcept { return creatures; }

	void Atmosphere(int a) noexcept { atmosphere = a; }
	int Atmosphere() const noexcept { return atmosphere; }
	bool HasAtmosphere() const noexcept { return atmosphere >= 0; }

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

	glm::dmat4 orbital;
	glm::dmat4 inverse_orbital;
	glm::dmat4 local;
	glm::dmat4 inverse_local;

	std::vector<creature::Creature *> creatures;
	int atmosphere;

};

}
}

#endif
