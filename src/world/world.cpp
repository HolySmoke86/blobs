#include "Body.hpp"
#include "Orbit.hpp"
#include "Planet.hpp"
#include "Simulation.hpp"
#include "Sun.hpp"
#include "Tile.hpp"
#include "TileSet.hpp"
#include "TileType.hpp"

#include "Creature.hpp"
#include "../const.hpp"
#include "../app/Assets.hpp"
#include "../graphics/Viewport.hpp"
#include "../rand/OctaveNoise.hpp"
#include "../rand/SimplexNoise.hpp"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/io.hpp>
#include <glm/gtx/transform.hpp>

using blobs::G;
using blobs::PI_2p0;

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
, surface_tilt(0.0, 0.0)
, axis_tilt(0.0, 0.0)
, rotation(0.0)
, angular(0.0)
, orbital(1.0)
, inverse_orbital(1.0)
, local(1.0)
, inverse_local(1.0)
, creatures() {
}

Body::~Body() {
	for (Creature *c : creatures) {
		delete c;
	}
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
		return PI_2p0 * sqrt(pow(orbit.SemiMajorAxis(), 3) / (G * (parent->Mass() + Mass())));
	} else {
		return 0.0;
	}
}

double Body::RotationalPeriod() const noexcept {
	if (std::abs(angular) < std::numeric_limits<double>::epsilon()) {
		return std::numeric_limits<double>::infinity();
	} else {
		return PI_2p0 * Inertia() / angular;
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

void Body::Cache() noexcept {
	if (parent) {
		orbital =
			orbit.Matrix(PI_2p0 * (GetSimulation().Time() / OrbitalPeriod()))
			* glm::eulerAngleXY(axis_tilt.x, axis_tilt.y);
		inverse_orbital =
			glm::eulerAngleYX(-axis_tilt.y, -axis_tilt.x)
			* orbit.InverseMatrix(PI_2p0 * (GetSimulation().Time() / OrbitalPeriod()));
	} else {
		orbital = glm::eulerAngleXY(axis_tilt.x, axis_tilt.y);
		inverse_orbital = glm::eulerAngleYX(-axis_tilt.y, -axis_tilt.x);
	}
	local =
		glm::eulerAngleY(rotation)
		* glm::eulerAngleXY(surface_tilt.x, surface_tilt.y);
	inverse_local =
		glm::eulerAngleYX(-surface_tilt.y, -surface_tilt.x)
		* glm::eulerAngleY(-rotation);
}

void Body::AddCreature(Creature *c) {
	c->SetBody(*this);
	creatures.push_back(c);
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
		if (abs(dE) < 1.0e-6) break;
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

glm::dvec3 Planet::TileCenter(int surface, int x, int y) const noexcept {
	glm::dvec3 center(0.0f);
	center[(surface + 0) % 3] = x + 0.5 - Radius();
	center[(surface + 1) % 3] = y + 0.5 - Radius();
	center[(surface + 2) % 3] = surface < 3 ? Radius() : -Radius();
	return center;
}

void Planet::BuildVAO(const TileSet &ts) {
	vao.Bind();
	vao.BindAttributes();
	vao.EnableAttribute(0);
	vao.EnableAttribute(1);
	vao.AttributePointer<glm::vec3>(0, false, offsetof(Attributes, position));
	vao.AttributePointer<glm::vec3>(1, false, offsetof(Attributes, tex_coord));
	vao.ReserveAttributes(TilesTotal() * 4, GL_STATIC_DRAW);
	{
		auto attrib = vao.MapAttributes(GL_WRITE_ONLY);
		float offset = Radius();

		// srf  0  1  2  3  4  5
		//  up +Z +X +Y -Z -X -Y

		for (int index = 0, surface = 0; surface < 6; ++surface) {
			for (int y = 0; y < sidelength; ++y) {
				for (int x = 0; x < sidelength; ++x, ++index) {
					float tex = ts[TileAt(surface, x, y).type].texture;
					const float tex_u_begin = surface < 3 ? 1.0f : 0.0f;
					const float tex_u_end = surface < 3 ? 0.0f : 1.0f;
					attrib[4 * index + 0].position[(surface + 0) % 3] = x + 0 - offset;
					attrib[4 * index + 0].position[(surface + 1) % 3] = y + 0 - offset;
					attrib[4 * index + 0].position[(surface + 2) % 3] = surface < 3 ? offset : -offset;
					attrib[4 * index + 0].tex_coord[0] = tex_u_begin;
					attrib[4 * index + 0].tex_coord[1] = 1.0f;
					attrib[4 * index + 0].tex_coord[2] = tex;

					attrib[4 * index + 1].position[(surface + 0) % 3] = x + 0 - offset;
					attrib[4 * index + 1].position[(surface + 1) % 3] = y + 1 - offset;
					attrib[4 * index + 1].position[(surface + 2) % 3] = surface < 3 ? offset : -offset;
					attrib[4 * index + 1].tex_coord[0] = tex_u_end;
					attrib[4 * index + 1].tex_coord[1] = 1.0f;
					attrib[4 * index + 1].tex_coord[2] = tex;

					attrib[4 * index + 2].position[(surface + 0) % 3] = x + 1 - offset;
					attrib[4 * index + 2].position[(surface + 1) % 3] = y + 0 - offset;
					attrib[4 * index + 2].position[(surface + 2) % 3] = surface < 3 ? offset : -offset;
					attrib[4 * index + 2].tex_coord[0] = tex_u_begin;
					attrib[4 * index + 2].tex_coord[1] = 0.0f;
					attrib[4 * index + 2].tex_coord[2] = tex;

					attrib[4 * index + 3].position[(surface + 0) % 3] = x + 1 - offset;
					attrib[4 * index + 3].position[(surface + 1) % 3] = y + 1 - offset;
					attrib[4 * index + 3].position[(surface + 2) % 3] = surface < 3 ? offset : -offset;
					attrib[4 * index + 3].tex_coord[0] = tex_u_end;
					attrib[4 * index + 3].tex_coord[1] = 0.0f;
					attrib[4 * index + 3].tex_coord[2] = tex;
				}
			}
		}
	}
	vao.BindElements();
	vao.ReserveElements(TilesTotal() * 6, GL_STATIC_DRAW);
	{
		auto element = vao.MapElements(GL_WRITE_ONLY);
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
	vao.Unbind();
}

void Planet::Draw(app::Assets &assets, graphics::Viewport &viewport) {
	vao.Bind();
	const glm::mat4 &MV = assets.shaders.planet_surface.MV();
	assets.shaders.planet_surface.SetNormal(glm::vec3(MV * glm::vec4(0.0f, 0.0f, 1.0f, 0.0f)));
	vao.DrawTriangles(TilesPerSurface() * 6, TilesPerSurface() * 6 * 0);
	assets.shaders.planet_surface.SetNormal(glm::vec3(MV * glm::vec4(1.0f, 0.0f, 0.0f, 0.0f)));
	vao.DrawTriangles(TilesPerSurface() * 6, TilesPerSurface() * 6 * 1);
	assets.shaders.planet_surface.SetNormal(glm::vec3(MV * glm::vec4(0.0f, 1.0f, 0.0f, 0.0f)));
	vao.DrawTriangles(TilesPerSurface() * 6, TilesPerSurface() * 6 * 2);
	assets.shaders.planet_surface.SetNormal(glm::vec3(MV * glm::vec4(0.0f, 0.0f, -1.0f, 0.0f)));
	vao.DrawTriangles(TilesPerSurface() * 6, TilesPerSurface() * 6 * 3);
	assets.shaders.planet_surface.SetNormal(glm::vec3(MV * glm::vec4(-1.0f, 0.0f, 0.0f, 0.0f)));
	vao.DrawTriangles(TilesPerSurface() * 6, TilesPerSurface() * 6 * 4);
	assets.shaders.planet_surface.SetNormal(glm::vec3(MV * glm::vec4(0.0f, -1.0f, 0.0f, 0.0f)));
	vao.DrawTriangles(TilesPerSurface() * 6, TilesPerSurface() * 6 * 5);
}


void GenerateEarthlike(const TileSet &tiles, Planet &p) noexcept {
	rand::SimplexNoise elevation_gen(0);

	const int ice = tiles["ice"].id;
	const int grass = tiles["grass"].id;
	const int water = tiles["water"].id;
	const int sand = tiles["sand"].id;
	const int rock = tiles["rock"].id;

	constexpr double water_thresh = 0.0;
	constexpr double beach_thresh = 0.1;
	constexpr double mountain_thresh = 0.5;

	const glm::dvec3 axis(glm::dvec4(0.0, 1.0, 0.0, 0.0) * glm::eulerAngleXY(p.SurfaceTilt().x, p.SurfaceTilt().y));
	const double cap_thresh = std::cos(p.AxialTilt().x);

	for (int surface = 0; surface <= 5; ++surface) {
		for (int y = 0; y < p.SideLength(); ++y) {
			for (int x = 0; x < p.SideLength(); ++x) {
				glm::dvec3 to_tile = p.TileCenter(surface, x, y);
				double near_axis = std::abs(glm::dot(glm::normalize(to_tile), axis));
				if (near_axis > cap_thresh) {
					p.TileAt(surface, x, y).type = ice;
					continue;
				}
				float elevation = rand::OctaveNoise(
					elevation_gen,
					to_tile / p.Radius(),
					3,   // octaves
					0.5, // persistence
					2 / p.Radius(), // frequency
					2,   // amplitude
					2    // growth
				);
				if (elevation < water_thresh) {
					p.TileAt(surface, x, y).type = water;
				} else if (elevation < beach_thresh) {
					p.TileAt(surface, x, y).type = sand;
				} else if (elevation < mountain_thresh) {
					p.TileAt(surface, x, y).type = grass;
				} else {
					p.TileAt(surface, x, y).type = rock;
				}
			}
		}
	}
	p.BuildVAO(tiles);
}

void GenerateTest(const TileSet &tiles, Planet &p) noexcept {
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
	p.BuildVAO(tiles);
}


Sun::Sun()
: Body() {
}

Sun::~Sun() {
}

TileSet::TileSet()
: types()
, names() {
}

TileSet::~TileSet() {
}

int TileSet::Add(const TileType &t) {
	int id = types.size();
	if (!names.emplace(t.name, id).second) {
		throw std::runtime_error("duplicate tile type name " + t.name);
	}
	types.emplace_back(t);
	types.back().id = id;
	return id;
}

TileType &TileSet::operator [](const std::string &name) {
	auto entry = names.find(name);
	if (entry != names.end()) {
		return types[entry->second];
	} else {
		throw std::runtime_error("unknown tile type " + name);
	}
}

const TileType &TileSet::operator [](const std::string &name) const {
	auto entry = names.find(name);
	if (entry != names.end()) {
		return types[entry->second];
	} else {
		throw std::runtime_error("unknown tile type " + name);
	}
}

}
}