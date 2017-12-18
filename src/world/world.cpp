#include "Body.hpp"
#include "CreatureCreatureCollision.hpp"
#include "Orbit.hpp"
#include "Planet.hpp"
#include "Resource.hpp"
#include "Set.hpp"
#include "Simulation.hpp"
#include "Sun.hpp"
#include "Tile.hpp"
#include "TileType.hpp"

#include "../app/Assets.hpp"
#include "../creature/Composition.hpp"
#include "../creature/Creature.hpp"
#include "../graphics/Viewport.hpp"
#include "../math/const.hpp"
#include "../math/geometry.hpp"
#include "../math/OctaveNoise.hpp"
#include "../math/SimplexNoise.hpp"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/io.hpp>
#include <glm/gtx/transform.hpp>

using std::sin;
using std::cos;
using std::pow;
using std::sqrt;


namespace blobs {
namespace world {

Body::Body()
: sim(nullptr)
, parent(nullptr)
, children()
, mass(1.0)
, radius(1.0)
, orbit()
, axis_tilt(0.0, 0.0)
, rotation(0.0)
, angular(0.0)
, orbital(1.0)
, inverse_orbital(1.0)
, local(1.0)
, inverse_local(1.0)
, creatures()
, atmosphere(-1) {
}

Body::~Body() {
}

void Body::SetSimulation(Simulation &s) noexcept {
	sim = &s;
	for (auto child : children) {
		child->SetSimulation(s);
	}
}

void Body::SetParent(Body &p) {
	if (HasParent()) {
		UnsetParent();
	}
	parent = &p;
	parent->AddChild(*this);
}

void Body::UnsetParent() {
	if (!HasParent()) return;
	parent->RemoveChild(*this);
	parent = nullptr;
}

void Body::AddChild(Body &c) {
	children.push_back(&c);
	c.SetSimulation(*sim);
}

void Body::RemoveChild(Body &c) {
	auto entry = std::find(children.begin(), children.end(), &c);
	if (entry != children.end()) {
		children.erase(entry);
	}
}

double Body::Inertia() const noexcept {
	// assume solid sphere for now
	return (2.0/5.0) * Mass() * pow(Radius(), 2);
}

double Body::GravitationalParameter() const noexcept {
	return G * Mass();
}

double Body::OrbitalPeriod() const noexcept {
	if (parent) {
		return PI * 2.0 * sqrt(pow(orbit.SemiMajorAxis(), 3) / (G * (parent->Mass() + Mass())));
	} else {
		return 0.0;
	}
}

double Body::RotationalPeriod() const noexcept {
	if (std::abs(angular) < std::numeric_limits<double>::epsilon()) {
		return std::numeric_limits<double>::infinity();
	} else {
		return PI * 2.0 * Inertia() / angular;
	}
}

double Body::DayLength() const noexcept {
	if (!HasParent()) {
		return RotationalPeriod();
	}
	double year = OrbitalPeriod();
	double sidereal = RotationalPeriod();
	double grade = (angular < 0.0 ? -1.0 : 1.0) * (std::abs(axis_tilt.x) > PI * 0.5 ? -1.0 : 1.0);
	return std::abs((year * sidereal) / ( year + (grade * sidereal)));
}

double Body::SphereOfInfluence() const noexcept {
	if (HasParent()) {
		return orbit.SemiMajorAxis() * std::pow(Mass() / Parent().Mass(), 2.0 / 5.0);
	} else {
		return std::numeric_limits<double>::infinity();
	}
}

glm::dmat4 Body::ToUniverse() const noexcept {
	glm::dmat4 m(1.0);
	const Body *b = this;
	while (b->HasParent()) {
		m = b->ToParent() * m;
		b = &b->Parent();
	}
	return m;
}

glm::dmat4 Body::FromUniverse() const noexcept {
	glm::dmat4 m(1.0);
	const Body *b = this;
	while (b->HasParent()) {
		m *= b->FromParent();
		b = &b->Parent();
	}
	return m;
}

namespace {
std::vector<creature::Creature *> ccache;
std::vector<CreatureCreatureCollision> collisions;
}

void Body::Tick(double dt) {
	rotation += dt * AngularMomentum() / Inertia();
	Cache();
	ccache = Creatures();
	for (creature::Creature *c : ccache) {
		c->Tick(dt);
	}
	// first remove creatures so they don't collide
	for (auto c = Creatures().begin(); c != Creatures().end();) {
		if ((*c)->Removable()) {
			(*c)->Removed();
			c = Creatures().erase(c);
		} else {
			++c;
		}
	}
	CheckCollision();
}

void Body::Cache() noexcept {
	if (parent) {
		orbital =
			orbit.Matrix(PI * 2.0 * (GetSimulation().Time() / OrbitalPeriod()))
			* glm::eulerAngleXY(axis_tilt.x, axis_tilt.y);
		inverse_orbital =
			glm::eulerAngleYX(-axis_tilt.y, -axis_tilt.x)
			* orbit.InverseMatrix(PI * 2.0 * (GetSimulation().Time() / OrbitalPeriod()));
	} else {
		orbital = glm::eulerAngleXY(axis_tilt.x, axis_tilt.y);
		inverse_orbital = glm::eulerAngleYX(-axis_tilt.y, -axis_tilt.x);
	}
	local = glm::eulerAngleY(rotation);
	inverse_local = glm::eulerAngleY(-rotation);
}

void Body::CheckCollision() noexcept {
	if (Creatures().size() < 2) return;
	collisions.clear();
	auto end = Creatures().end();
	for (auto i = Creatures().begin(); i != end; ++i) {
		math::AABB i_box((*i)->CollisionBounds());
		glm::dmat4 i_mat((*i)->CollisionTransform());
		for (auto j = (i + 1); j != end; ++j) {
			glm::dvec3 diff((*i)->GetSituation().Position() - (*j)->GetSituation().Position());
			double max_dist = ((*i)->Size() + (*j)->Size()) * 1.74;
			if (glm::length2(diff) > max_dist * max_dist) continue;
			math::AABB j_box((*j)->CollisionBounds());
			glm::dmat4 j_mat((*j)->CollisionTransform());
			glm::dvec3 normal;
			double depth;
			if (Intersect(i_box, i_mat, j_box, j_mat, normal, depth)) {
				collisions.push_back({ **i, **j, normal, depth });
			}
		}
	}
	for (auto &c : collisions) {
		c.A().OnCollide(c.B());
		c.B().OnCollide(c.A());
		c.A().GetSituation().Move(c.Normal() * (c.Depth() * -0.5));
		c.B().GetSituation().Move(c.Normal() * (c.Depth() * 0.5));
		c.A().GetSituation().Accelerate(c.Normal() * -glm::dot(c.Normal(), c.AVel()));
		c.B().GetSituation().Accelerate(c.Normal() * -glm::dot(c.Normal(), c.BVel()));
	}
}

void Body::AddCreature(creature::Creature *c) {
	creatures.push_back(c);
}

void Body::RemoveCreature(creature::Creature *c) {
	auto entry = std::find(creatures.begin(), creatures.end(), c);
	if (entry != creatures.end()) {
		creatures.erase(entry);
	}
}


CreatureCreatureCollision::~CreatureCreatureCollision() {
}

const glm::dvec3 &CreatureCreatureCollision::APos() const noexcept {
	return a->GetSituation().Position();
}

const glm::dvec3 &CreatureCreatureCollision::AVel() const noexcept {
	return a->GetSituation().Velocity();
}

const glm::dvec3 &CreatureCreatureCollision::BPos() const noexcept {
	return b->GetSituation().Position();
}

const glm::dvec3 &CreatureCreatureCollision::BVel() const noexcept {
	return b->GetSituation().Velocity();
}


Orbit::Orbit()
: sma(1.0)
, ecc(0.0)
, inc(0.0)
, asc(0.0)
, arg(0.0)
, mna(0.0) {
}

Orbit::~Orbit() {
}

double Orbit::SemiMajorAxis() const noexcept {
	return sma;
}

Orbit &Orbit::SemiMajorAxis(double s) noexcept {
	sma = s;
	return *this;
}

double Orbit::Eccentricity() const noexcept {
	return ecc;
}

Orbit &Orbit::Eccentricity(double e) noexcept {
	ecc = e;
	return *this;
}

double Orbit::Inclination() const noexcept {
	return inc;
}

Orbit &Orbit::Inclination(double i) noexcept {
	inc = i;
	return *this;
}

double Orbit::LongitudeAscending() const noexcept {
	return asc;
}

Orbit &Orbit::LongitudeAscending(double l) noexcept {
	asc = l;
	return *this;
}

double Orbit::ArgumentPeriapsis() const noexcept {
	return arg;
}

Orbit &Orbit::ArgumentPeriapsis(double a) noexcept {
	arg = a;
	return *this;
}

double Orbit::MeanAnomaly() const noexcept {
	return mna;
}

Orbit &Orbit::MeanAnomaly(double m) noexcept {
	mna = m;
	return *this;
}

namespace {

double mean2eccentric(double M, double e) {
	double E = M; // eccentric anomaly, solve M = E - e sin E
	// limit to 100 steps to prevent deadlocks in impossible situations
	for (int i = 0; i < 100; ++i) {
		double dE = (E - e * sin(E) - M) / (1 - e * cos(E));
		E -= dE;
		if (std::abs(dE) < 1.0e-6) break;
	}
	return E;
}

}

glm::dmat4 Orbit::Matrix(double t) const noexcept {
	double M = mna + t;
	double E = mean2eccentric(M, ecc);

	// coordinates in orbital plane, P=x, Q=-z
	double P = sma * (cos(E) - ecc);
	double Q = sma * sin(E) * sqrt(1 - (ecc * ecc));

	return glm::yawPitchRoll(asc, inc, arg) * glm::translate(glm::dvec3(P, 0.0, -Q));
}

glm::dmat4 Orbit::InverseMatrix(double t) const noexcept {
	double M = mna + t;
	double E = mean2eccentric(M, ecc);
	double P = sma * (cos(E) - ecc);
	double Q = sma * sin(E) * sqrt(1 - (ecc * ecc));
	return glm::translate(glm::dvec3(-P, 0.0, Q)) * glm::transpose(glm::yawPitchRoll(asc, inc, arg));
}


Planet::Planet(int sidelength)
: Body()
, sidelength(sidelength)
, tiles(TilesTotal())
, vao() {
	Radius(double(sidelength) / 2.0);
}

Planet::~Planet() {
}

namespace {
/// map p onto cube, s gives the surface, u and v the position in [-1,1]
void cubemap(const glm::dvec3 &p, int &s, double &u, double &v) noexcept {
	const glm::dvec3 p_abs(glm::abs(p));
	const glm::bvec3 p_pos(glm::greaterThan(p, glm::dvec3(0.0)));
	double max_axis = 0.0;

	if (p_pos.x && p_abs.x >= p_abs.y && p_abs.x >= p_abs.z) {
		max_axis = p_abs.x;
		u = p.y;
		v = p.z;
		s = 1;
	}
	if (!p_pos.x && p_abs.x >= p_abs.y && p_abs.x >= p_abs.z) {
		max_axis = p_abs.x;
		u = -p.y;
		v = -p.z;
		s = 4;
	}
	if (p_pos.y && p_abs.y >= p_abs.x && p_abs.y >= p_abs.z) {
		max_axis = p_abs.y;
		u = p.z;
		v = p.x;
		s = 2;
	}
	if (!p_pos.y && p_abs.y >= p_abs.x && p_abs.y >= p_abs.z) {
		max_axis = p_abs.y;
		u = -p.z;
		v = -p.x;
		s = 5;
	}
	if (p_pos.z && p_abs.z >= p_abs.x && p_abs.z >= p_abs.y) {
		max_axis = p_abs.z;
		u = p.x;
		v = p.y;
		s = 0;
	}
	if (!p_pos.z && p_abs.z >= p_abs.x && p_abs.z >= p_abs.y) {
		max_axis = p_abs.z;
		u = -p.x;
		v = -p.y;
		s = 3;
	}
	u /= max_axis;
	v /= max_axis;
}
/// get p from cube, s being surface, u and v the position in [-1,1],
/// gives a vector from the center to the surface
glm::dvec3 cubeunmap(int s, double u, double v) {
	switch (s) {
		default:
		case 0: return glm::dvec3(u, v, 1.0); // +Z
		case 1: return glm::dvec3(1.0, u, v); // +X
		case 2: return glm::dvec3(v, 1.0, u); // +Y
		case 3: return glm::dvec3(-u, -v, -1.0); // -Z
		case 4: return glm::dvec3(-1.0, -u, -v); // -X
		case 5: return glm::dvec3(-v, -1.0, -u); // -Y
	};
}
}

Tile &Planet::TileAt(const glm::dvec3 &p) noexcept {
	int srf = 0;
	double u = 0.0;
	double v = 0.0;
	cubemap(p, srf, u, v);
	int x = glm::clamp(int(u * Radius() + Radius()), 0, sidelength - 1);
	int y = glm::clamp(int(v * Radius() + Radius()), 0, sidelength - 1);
	return TileAt(srf, x, y);
}

const Tile &Planet::TileAt(const glm::dvec3 &p) const noexcept {
	int srf = 0;
	double u = 0.0;
	double v = 0.0;
	cubemap(p, srf, u, v);
	int x = glm::clamp(int(u * Radius() + Radius()), 0, sidelength - 1);
	int y = glm::clamp(int(v * Radius() + Radius()), 0, sidelength - 1);
	return TileAt(srf, x, y);
}

const TileType &Planet::TileTypeAt(const glm::dvec3 &p) const noexcept {
	return GetSimulation().TileTypes()[TileAt(p).type];
}

Tile &Planet::TileAt(int surface, int x, int y) noexcept {
	return tiles[IndexOf(surface, x, y)];
}

const Tile &Planet::TileAt(int surface, int x, int y) const noexcept {
	return tiles[IndexOf(surface, x, y)];
}

const TileType &Planet::TypeAt(int srf, int x, int y) const noexcept {
	return GetSimulation().TileTypes()[TileAt(srf, x, y).type];
}

glm::dvec3 Planet::TileCenter(int srf, int x, int y, double e) const noexcept {
	double u = (double(x) - Radius() + 0.5) / Radius();
	double v = (double(y) - Radius() + 0.5) / Radius();
	return glm::normalize(cubeunmap(srf, u, v)) * (Radius() + e);
}

void Planet::BuildVAO() {
	vao.reset(new graphics::SimpleVAO<Attributes, unsigned int>);
	vao->Bind();
	vao->BindAttributes();
	vao->EnableAttribute(0);
	vao->EnableAttribute(1);
	vao->EnableAttribute(2);
	vao->EnableAttribute(3);
	vao->EnableAttribute(4);
	vao->EnableAttribute(5);
	vao->AttributePointer<glm::vec3>(0, false, offsetof(Attributes, position));
	vao->AttributePointer<glm::vec3>(1, false, offsetof(Attributes, normal));
	vao->AttributePointer<glm::vec3>(2, false, offsetof(Attributes, tex_coord));
	vao->AttributePointer<float>(3, false, offsetof(Attributes, shiny));
	vao->AttributePointer<float>(4, false, offsetof(Attributes, glossy));
	vao->AttributePointer<float>(5, false, offsetof(Attributes, metallic));
	vao->ReserveAttributes(TilesTotal() * 4, GL_STATIC_DRAW);
	{
		auto attrib = vao->MapAttributes(GL_WRITE_ONLY);
		float offset = Radius();

		// srf  0  1  2  3  4  5
		//  up +Z +X +Y -Z -X -Y

		for (int index = 0, surface = 0; surface < 6; ++surface) {
			for (int y = 0; y < sidelength; ++y) {
				for (int x = 0; x < sidelength; ++x, ++index) {
					glm::vec3 pos[4];
					pos[0][(surface + 0) % 3] = float(x + 0) - offset;
					pos[0][(surface + 1) % 3] = float(y + 0) - offset;
					pos[0][(surface + 2) % 3] = offset;
					pos[1][(surface + 0) % 3] = float(x + 0) - offset;
					pos[1][(surface + 1) % 3] = float(y + 1) - offset;
					pos[1][(surface + 2) % 3] = offset;
					pos[2][(surface + 0) % 3] = float(x + 1) - offset;
					pos[2][(surface + 1) % 3] = float(y + 0) - offset;
					pos[2][(surface + 2) % 3] = offset;
					pos[3][(surface + 0) % 3] = float(x + 1) - offset;
					pos[3][(surface + 1) % 3] = float(y + 1) - offset;
					pos[3][(surface + 2) % 3] = offset;

					const TileType &t = TypeAt(surface, x, y);
					float tex = t.texture;
					const float tex_v_begin = surface < 3 ? 1.0f : 0.0f;
					const float tex_v_end = surface < 3 ? 0.0f : 1.0f;

					attrib[4 * index + 0].position = glm::normalize(pos[0]) * (surface < 3 ? offset : -offset);
					attrib[4 * index + 0].normal = pos[0];
					attrib[4 * index + 0].tex_coord[0] = 0.0f;
					attrib[4 * index + 0].tex_coord[1] = tex_v_begin;
					attrib[4 * index + 0].tex_coord[2] = tex;
					attrib[4 * index + 0].shiny = t.shiny;
					attrib[4 * index + 0].glossy = t.glossy;
					attrib[4 * index + 0].metallic = t.metallic;

					attrib[4 * index + 1].position = glm::normalize(pos[1]) * (surface < 3 ? offset : -offset);
					attrib[4 * index + 1].normal = pos[1];
					attrib[4 * index + 1].tex_coord[0] = 0.0f;
					attrib[4 * index + 1].tex_coord[1] = tex_v_end;
					attrib[4 * index + 1].tex_coord[2] = tex;
					attrib[4 * index + 1].shiny = t.shiny;
					attrib[4 * index + 1].glossy = t.glossy;
					attrib[4 * index + 1].metallic = t.metallic;

					attrib[4 * index + 2].position = glm::normalize(pos[2]) * (surface < 3 ? offset : -offset);
					attrib[4 * index + 2].normal = pos[2];
					attrib[4 * index + 2].tex_coord[0] = 1.0f;
					attrib[4 * index + 2].tex_coord[1] = tex_v_begin;
					attrib[4 * index + 2].tex_coord[2] = tex;
					attrib[4 * index + 2].shiny = t.shiny;
					attrib[4 * index + 2].glossy = t.glossy;
					attrib[4 * index + 2].metallic = t.metallic;

					attrib[4 * index + 3].position = glm::normalize(pos[3]) * (surface < 3 ? offset : -offset);
					attrib[4 * index + 3].normal = pos[3];
					attrib[4 * index + 3].tex_coord[0] = 1.0f;
					attrib[4 * index + 3].tex_coord[1] = tex_v_end;
					attrib[4 * index + 3].tex_coord[2] = tex;
					attrib[4 * index + 3].shiny = t.shiny;
					attrib[4 * index + 3].glossy = t.glossy;
					attrib[4 * index + 3].metallic = t.metallic;
				}
			}
		}
	}
	vao->BindElements();
	vao->ReserveElements(TilesTotal() * 6, GL_STATIC_DRAW);
	{
		auto element = vao->MapElements(GL_WRITE_ONLY);
		int index = 0;
		for (int surface = 0; surface < 3; ++surface) {
			for (int y = 0; y < sidelength; ++y) {
				for (int x = 0; x < sidelength; ++x, ++index) {
					element[6 * index + 0] = 4 * index + 0;
					element[6 * index + 1] = 4 * index + 2;
					element[6 * index + 2] = 4 * index + 1;
					element[6 * index + 3] = 4 * index + 1;
					element[6 * index + 4] = 4 * index + 2;
					element[6 * index + 5] = 4 * index + 3;
				}
			}
		}
		for (int surface = 3; surface < 6; ++surface) {
			for (int y = 0; y < sidelength; ++y) {
				for (int x = 0; x < sidelength; ++x, ++index) {
					element[6 * index + 0] = 4 * index + 0;
					element[6 * index + 1] = 4 * index + 1;
					element[6 * index + 2] = 4 * index + 2;
					element[6 * index + 3] = 4 * index + 2;
					element[6 * index + 4] = 4 * index + 1;
					element[6 * index + 5] = 4 * index + 3;
				}
			}
		}
	}
	vao->Unbind();
}

void Planet::Draw(app::Assets &assets, graphics::Viewport &viewport) {
	if (!vao) return;

	vao->Bind();
	vao->DrawTriangles(TilesTotal() * 6);
}


void GenerateEarthlike(const Set<TileType> &tiles, Planet &p) noexcept {
	math::SimplexNoise elevation_gen(0);
	math::SimplexNoise variation_gen(45623752346);

	const int ice = tiles["ice"].id;
	const int ocean = tiles["ocean"].id;
	const int water = tiles["water"].id;
	const int sand = tiles["sand"].id;
	const int grass = tiles["grass"].id;
	const int tundra = tiles["tundra"].id;
	const int taiga = tiles["taiga"].id;
	const int desert = tiles["desert"].id;
	const int mntn = tiles["mountain"].id;
	const int algae = tiles["algae"].id;
	const int forest = tiles["forest"].id;
	const int jungle = tiles["jungle"].id;
	const int rock = tiles["rock"].id;
	const int wheat = tiles["wheat"].id;

	constexpr double ocean_thresh = -0.2;
	constexpr double water_thresh = 0.0;
	constexpr double beach_thresh = 0.05;
	constexpr double highland_thresh = 0.4;
	constexpr double mountain_thresh = 0.5;

	const glm::dvec3 axis(0.0, 1.0, 0.0);
	const double cap_thresh = std::abs(std::cos(p.AxialTilt().x));
	const double equ_thresh = std::abs(std::sin(p.AxialTilt().x)) / 2.0;
	const double fzone_start = equ_thresh - (equ_thresh - cap_thresh) / 3.0;
	const double fzone_end = cap_thresh + (equ_thresh - cap_thresh) / 3.0;

	for (int surface = 0; surface <= 5; ++surface) {
		for (int y = 0; y < p.SideLength(); ++y) {
			for (int x = 0; x < p.SideLength(); ++x) {
				glm::dvec3 to_tile = p.TileCenter(surface, x, y);
				double near_axis = std::abs(glm::dot(glm::normalize(to_tile), axis));
				if (near_axis > cap_thresh) {
					p.TileAt(surface, x, y).type = ice;
					continue;
				}
				float elevation = math::OctaveNoise(
					elevation_gen,
					glm::vec3(to_tile / p.Radius()),
					3,   // octaves
					0.5, // persistence
					5 / p.Radius(), // frequency
					2,   // amplitude
					2    // growth
				);
				float variation = math::OctaveNoise(
					variation_gen,
					glm::vec3(to_tile / p.Radius()),
					3,   // octaves
					0.5, // persistence
					16 / p.Radius(), // frequency
					2,   // amplitude
					2    // growth
				);
				if (elevation < ocean_thresh) {
					p.TileAt(surface, x, y).type = ocean;
				} else if (elevation < water_thresh) {
					if (variation > 0.3) {
						p.TileAt(surface, x, y).type = algae;
					} else {
						p.TileAt(surface, x, y).type = water;
					}
				} else if (elevation < beach_thresh) {
					p.TileAt(surface, x, y).type = sand;
				} else if (elevation < highland_thresh) {
					if (near_axis < equ_thresh) {
						if (variation > 0.6) {
							p.TileAt(surface, x, y).type = grass;
						} else if (variation > 0.2) {
							p.TileAt(surface, x, y).type = sand;
						} else {
							p.TileAt(surface, x, y).type = desert;
						}
					} else if (near_axis < fzone_start) {
						if (variation > 0.4) {
							p.TileAt(surface, x, y).type = forest;
						} else if (variation < -0.5) {
							p.TileAt(surface, x, y).type = jungle;
						} else if (variation > -0.02 && variation < 0.02) {
							p.TileAt(surface, x, y).type = wheat;
						} else {
							p.TileAt(surface, x, y).type = grass;
						}
					} else if (near_axis < fzone_end) {
						p.TileAt(surface, x, y).type = tundra;
					} else {
						p.TileAt(surface, x, y).type = taiga;
					}
				} else if (elevation < mountain_thresh) {
					if (variation > 0.3) {
						p.TileAt(surface, x, y).type = mntn;
					} else {
						p.TileAt(surface, x, y).type = rock;
					}
				} else {
					p.TileAt(surface, x, y).type = mntn;
				}
			}
		}
	}
	p.BuildVAO();
}

void GenerateTest(const Set<TileType> &tiles, Planet &p) noexcept {
	for (int surface = 0; surface <= 5; ++surface) {
		for (int y = 0; y < p.SideLength(); ++y) {
			for (int x = 0; x < p.SideLength(); ++x) {
				if (x == p.SideLength() / 2 && y == p.SideLength() / 2) {
					p.TileAt(surface, x, y).type = surface;
				} else {
					p.TileAt(surface, x, y).type = (x == p.SideLength()/2) + (y == p.SideLength()/2) + 6;
				}
			}
		}
	}
	p.BuildVAO();
}


Sun::Sun()
: Body()
, color(1.0)
, luminosity(1.0) {
}

Sun::~Sun() {
}


std::vector<TileType::Yield>::const_iterator TileType::FindResource(int r) const {
	auto yield = resources.cbegin();
	for (; yield != resources.cend(); ++yield) {
		if (yield->resource == r) {
			break;
		}
	}
	return yield;
}

std::vector<TileType::Yield>::const_iterator TileType::FindBestResource(const creature::Composition &comp) const {
	auto best = resources.cend();
	double best_value = 0.0;
	for (auto yield = resources.cbegin(); yield != resources.cend(); ++yield) {
		double value = comp.Get(yield->resource);
		if (value > best_value) {
			best = yield;
			best_value = value;
		}
	}
	return best;
}

}
}
