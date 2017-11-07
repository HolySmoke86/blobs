#include "Body.hpp"
#include "Planet.hpp"
#include "Simulation.hpp"
#include "Sun.hpp"
#include "Tile.hpp"

#include "../const.hpp"
#include "../app/Assets.hpp"
#include "../graphics/Viewport.hpp"

#include <algorithm>
#include <cmath>
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
, sma(1.0)
, ecc(0.0)
, inc(0.0)
, asc(0.0)
, arg(0.0)
, mna(0.0) {
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

double Body::Mass() const noexcept {
	return mass;
}

void Body::Mass(double m) noexcept {
	mass = m;
}

double Body::Radius() const noexcept {
	return radius;
}

void Body::Radius(double r) noexcept {
	radius = r;
}

double Body::SemiMajorAxis() const noexcept {
	return sma;
}

void Body::SemiMajorAxis(double s) noexcept {
	sma = s;
}

double Body::Eccentricity() const noexcept {
	return ecc;
}

void Body::Eccentricity(double e) noexcept {
	ecc = e;
}

double Body::Inclination() const noexcept {
	return inc;
}

void Body::Inclination(double i) noexcept {
	inc = i;
}

double Body::LongitudeAscending() const noexcept {
	return asc;
}

void Body::LongitudeAscending(double l) noexcept {
	asc = l;
}

double Body::ArgumentPeriapsis() const noexcept {
	return arg;
}

void Body::ArgumentPeriapsis(double a) noexcept {
	arg = a;
}

double Body::MeanAnomaly() const noexcept {
	return mna;
}

void Body::MeanAnomaly(double m) noexcept {
	mna = m;
}

double Body::GravitationalParameter() const noexcept {
	return G * Mass();
}

double Body::OrbitalPeriod() const noexcept {
	if (parent) {
		return PI_2p0 * sqrt((sma * sma * sma) / (G * (parent->Mass() + Mass())));
	} else {
		return 0.0;
	}
}

glm::mat4 Body::ToParent() const noexcept {
	if (!parent) {
		return glm::mat4(1.0f);
	}

	double T = OrbitalPeriod();

	double M = mna + PI_2p0 * (GetSimulation().Time() / T); // + time

	double E = M; // eccentric anomaly, solve M = E - e sin E
	while (true) {
		double dE = (E - ecc * sin(E) - M) / (1 - ecc * cos(E));
		E -= dE;
		if (abs(dE) < 1.0e-6) break;
	}

	// coordinates in orbital plane
	double P = sma * (cos(E) - ecc);
	double Q = sma * sin(E) * sqrt(1 - (ecc * ecc));

	// tile by argument of periapsis, …
	double x = cos(arg) * P - sin(arg) * Q;
	double y = sin(arg) * P + cos(arg) * Q;
	// …inclination, …
	double z = sin(inc) * x;
	       x = cos(inc) * x;
	// …and longitude of ascending node
	glm::vec3 pos(
		cos(asc) * x - sin(asc) * y,
		sin(asc) * x + cos(asc) * y,
		z);

	// TODO: calculate complete matrix
	return glm::translate(-pos);
}

glm::mat4 Body::FromParent() const noexcept {
	if (!parent) {
		return glm::mat4(1.0f);
	}
	// TODO: calculate real position
	return glm::translate(glm::vec3(-sma, 0.0f, 0.0f));
}


Planet::Planet(int sidelength)
: Body()
, sidelength(sidelength)
, tiles(new Tile[TilesTotal()])
, vao() {
}

Planet::~Planet() {
}

void Planet::BuildVAOs() {
	vao.Bind();
	vao.BindAttributes();
	vao.EnableAttribute(0);
	vao.EnableAttribute(1);
	vao.AttributePointer<glm::vec3>(0, false, offsetof(Attributes, position));
	vao.AttributePointer<glm::vec3>(1, false, offsetof(Attributes, tex_coord));
	vao.ReserveAttributes(TilesTotal() * 4, GL_STATIC_DRAW);
	{
		auto attrib = vao.MapAttributes(GL_WRITE_ONLY);
		float offset = sidelength * 0.5f;

		for (int index = 0, surface = 0; surface < 6; ++surface) {
			for (int y = 0; y < sidelength; ++y) {
				for (int x = 0; x < sidelength; ++x, ++index) {
					float tex = TileAt(surface, x, y).type;
					attrib[4 * index + 0].position[(surface + 0) % 3] = x + 0 - offset;
					attrib[4 * index + 0].position[(surface + 1) % 3] = y + 0 - offset;
					attrib[4 * index + 0].position[(surface + 2) % 3] = surface < 3 ? offset : -offset;
					attrib[4 * index + 0].tex_coord[0] = 0.0f;
					attrib[4 * index + 0].tex_coord[1] = 0.0f;
					attrib[4 * index + 0].tex_coord[2] = tex;

					attrib[4 * index + 1].position[(surface + 0) % 3] = x + 0 - offset;
					attrib[4 * index + 1].position[(surface + 1) % 3] = y + 1 - offset;
					attrib[4 * index + 1].position[(surface + 2) % 3] = surface < 3 ? offset : -offset;
					attrib[4 * index + 1].tex_coord[0] = 0.0f;
					attrib[4 * index + 1].tex_coord[1] = 1.0f;
					attrib[4 * index + 1].tex_coord[2] = tex;

					attrib[4 * index + 2].position[(surface + 0) % 3] = x + 1 - offset;
					attrib[4 * index + 2].position[(surface + 1) % 3] = y + 0 - offset;
					attrib[4 * index + 2].position[(surface + 2) % 3] = surface < 3 ? offset : -offset;
					attrib[4 * index + 2].tex_coord[0] = 1.0f;
					attrib[4 * index + 2].tex_coord[1] = 0.0f;
					attrib[4 * index + 2].tex_coord[2] = tex;

					attrib[4 * index + 3].position[(surface + 0) % 3] = x + 1 - offset;
					attrib[4 * index + 3].position[(surface + 1) % 3] = y + 1 - offset;
					attrib[4 * index + 3].position[(surface + 2) % 3] = surface < 3 ? offset : -offset;
					attrib[4 * index + 3].tex_coord[0] = 1.0f;
					attrib[4 * index + 3].tex_coord[1] = 1.0f;
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


void GenerateTest(Planet &p) {
	for (int surface = 0; surface <= 5; ++surface) {
		for (int y = 0; y < p.SideLength(); ++y) {
			for (int x = 0; x < p.SideLength(); ++x) {
				p.TileAt(surface, x, y).type = (x == p.SideLength()/2) + (y == p.SideLength()/2);
			}
		}
	}
	p.BuildVAOs();
}


Sun::Sun()
: Body() {
}

Sun::~Sun() {
}

}
}
