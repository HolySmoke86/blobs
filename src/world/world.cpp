#include "Body.hpp"
#include "Planet.hpp"
#include "Sun.hpp"
#include "Tile.hpp"

#include "../app/Assets.hpp"
#include "../graphics/Viewport.hpp"

#include <algorithm>


namespace blobs {
namespace world {

Body::Body()
: parent(nullptr)
, children()
, mass(1.0)
, radius(1.0) {
}

Body::~Body() {
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
}

void Body::RemoveChild(Body &c) {
	auto entry = std::find(children.begin(), children.end(), &c);
	if (entry != children.end()) {
		children.erase(entry);
	}
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
		for (int index = 0, surface = 0; surface < 6; ++surface) {
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
	// TODO: premultiply normal with model matrix (i.e. just take it from M)
	assets.shaders.planet_surface.SetNormal(glm::vec3(0.0f, 0.0f, 1.0f));
	vao.DrawTriangles(TilesTotal() * 4, TilesTotal() * 4 * 0);
	assets.shaders.planet_surface.SetNormal(glm::vec3(1.0f, 0.0f, 0.0f));
	vao.DrawTriangles(TilesTotal() * 4, TilesTotal() * 4 * 1);
	assets.shaders.planet_surface.SetNormal(glm::vec3(0.0f, 1.0f, 0.0f));
	vao.DrawTriangles(TilesTotal() * 4, TilesTotal() * 4 * 2);
	assets.shaders.planet_surface.SetNormal(glm::vec3(0.0f, 0.0f, -1.0f));
	vao.DrawTriangles(TilesTotal() * 4, TilesTotal() * 4 * 3);
	assets.shaders.planet_surface.SetNormal(glm::vec3(-1.0f, 0.0f, 0.0f));
	vao.DrawTriangles(TilesTotal() * 4, TilesTotal() * 4 * 4);
	assets.shaders.planet_surface.SetNormal(glm::vec3(0.0f, -1.0f, 0.0f));
	vao.DrawTriangles(TilesTotal() * 4, TilesTotal() * 4 * 5);
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
