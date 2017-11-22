#include "Creature.hpp"
#include "Situation.hpp"

#include "InhaleNeed.hpp"
#include "IngestNeed.hpp"
#include "Need.hpp"
#include "../app/Assets.hpp"
#include "../world/Body.hpp"
#include "../world/Planet.hpp"
#include "../world/Simulation.hpp"
#include "../world/TileType.hpp"

#include <glm/gtx/transform.hpp>

#include <iostream>


namespace blobs {
namespace creature {

Creature::Creature()
: name()
, health(1.0)
, needs()
, situation()
, vao() {
}

Creature::~Creature() {
}

void Creature::Hurt(double dt) noexcept {
	health = std::max(0.0, health - dt);
}

void Creature::Tick(double dt) {
	// update needs
	for (auto &need : needs) {
		need->Tick(dt);
	}
	// do background stuff
	for (auto &need : needs) {
		need->ApplyEffect(*this, dt);
	}
}

glm::dmat4 Creature::LocalTransform() noexcept {
	// TODO: surface transform
	constexpr double half_height = 0.25;
	const glm::dvec3 &pos = situation.Position();
	return glm::translate(glm::dvec3(pos.x, pos.y, pos.z + situation.GetPlanet().Radius() + half_height))
		* glm::scale(glm::dvec3(half_height, half_height, half_height));
}

void Creature::BuildVAO() {
	vao.Bind();
	vao.BindAttributes();
	vao.EnableAttribute(0);
	vao.EnableAttribute(1);
	vao.EnableAttribute(2);
	vao.AttributePointer<glm::vec3>(0, false, offsetof(Attributes, position));
	vao.AttributePointer<glm::vec3>(1, false, offsetof(Attributes, normal));
	vao.AttributePointer<glm::vec3>(2, false, offsetof(Attributes, texture));
	vao.ReserveAttributes(6 * 4, GL_STATIC_DRAW);
	{
		auto attrib = vao.MapAttributes(GL_WRITE_ONLY);
		const float offset = 1.0f;
		for (int surface = 0; surface < 6; ++surface) {
			const float tex_u_begin = surface < 3 ? 1.0f : 0.0f;
			const float tex_u_end = surface < 3 ? 0.0f : 1.0f;

			attrib[4 * surface + 0].position[(surface + 0) % 3] = -offset;
			attrib[4 * surface + 0].position[(surface + 1) % 3] = -offset;
			attrib[4 * surface + 0].position[(surface + 2) % 3] = surface < 3 ? offset : -offset;
			attrib[4 * surface + 0].normal[(surface + 0) % 3] = 0.0f;
			attrib[4 * surface + 0].normal[(surface + 1) % 3] = 0.0f;
			attrib[4 * surface + 0].normal[(surface + 2) % 3] = surface < 3 ? 1.0f : -1.0f;
			attrib[4 * surface + 0].texture.x = tex_u_begin;
			attrib[4 * surface + 0].texture.y = 1.0f;
			attrib[4 * surface + 0].texture.z = surface;

			attrib[4 * surface + 1].position[(surface + 0) % 3] = -offset;
			attrib[4 * surface + 1].position[(surface + 1) % 3] =  offset;
			attrib[4 * surface + 1].position[(surface + 2) % 3] = surface < 3 ? offset : -offset;
			attrib[4 * surface + 1].normal[(surface + 0) % 3] = 0.0f;
			attrib[4 * surface + 1].normal[(surface + 1) % 3] = 0.0f;
			attrib[4 * surface + 1].normal[(surface + 2) % 3] = surface < 3 ? 1.0f : -1.0f;
			attrib[4 * surface + 1].texture.x = tex_u_end;
			attrib[4 * surface + 1].texture.y = 1.0f;
			attrib[4 * surface + 1].texture.z = surface;

			attrib[4 * surface + 2].position[(surface + 0) % 3] =  offset;
			attrib[4 * surface + 2].position[(surface + 1) % 3] = -offset;
			attrib[4 * surface + 2].position[(surface + 2) % 3] = surface < 3 ? offset : -offset;
			attrib[4 * surface + 2].normal[(surface + 0) % 3] = 0.0f;
			attrib[4 * surface + 2].normal[(surface + 1) % 3] = 0.0f;
			attrib[4 * surface + 2].normal[(surface + 2) % 3] = surface < 3 ? 1.0f : -1.0f;
			attrib[4 * surface + 2].texture.x = tex_u_begin;
			attrib[4 * surface + 2].texture.y = 0.0f;
			attrib[4 * surface + 2].texture.z = surface;

			attrib[4 * surface + 3].position[(surface + 0) % 3] = offset;
			attrib[4 * surface + 3].position[(surface + 1) % 3] = offset;
			attrib[4 * surface + 3].position[(surface + 2) % 3] = surface < 3 ? offset : -offset;
			attrib[4 * surface + 3].normal[(surface + 0) % 3] = 0.0f;
			attrib[4 * surface + 3].normal[(surface + 1) % 3] = 0.0f;
			attrib[4 * surface + 3].normal[(surface + 2) % 3] = surface < 3 ? 1.0f : -1.0f;
			attrib[4 * surface + 3].texture.x = tex_u_end;
			attrib[4 * surface + 3].texture.y = 0.0f;
			attrib[4 * surface + 3].texture.z = surface;
		}
	}
	vao.BindElements();
	vao.ReserveElements(6 * 6, GL_STATIC_DRAW);
	{
		auto element = vao.MapElements(GL_WRITE_ONLY);
		for (int surface = 0; surface < 3; ++surface) {
			element[6 * surface + 0] = 4 * surface + 0;
			element[6 * surface + 1] = 4 * surface + 2;
			element[6 * surface + 2] = 4 * surface + 1;
			element[6 * surface + 3] = 4 * surface + 1;
			element[6 * surface + 4] = 4 * surface + 2;
			element[6 * surface + 5] = 4 * surface + 3;
		}
		for (int surface = 3; surface < 6; ++surface) {
			element[6 * surface + 0] = 4 * surface + 0;
			element[6 * surface + 1] = 4 * surface + 1;
			element[6 * surface + 2] = 4 * surface + 2;
			element[6 * surface + 3] = 4 * surface + 2;
			element[6 * surface + 4] = 4 * surface + 1;
			element[6 * surface + 5] = 4 * surface + 3;
		}
	}
	vao.Unbind();
}

void Creature::Draw(app::Assets &assets, graphics::Viewport &viewport) {
	vao.Bind();
	vao.DrawTriangles(6 * 6);
}


void Spawn(Creature &c, world::Planet &p, app::Assets &assets) {
	p.AddCreature(&c);
	c.GetSituation().SetPlanetSurface(p, 0, glm::dvec3(0.0, 0.0, 0.0));

	// probe surrounding area for common resources
	int start = p.SideLength() / 2 - 2;
	int end = start + 5;
	std::map<int, double> yields;
	for (int y = start; y < end; ++y) {
		for (int x = start; x < end; ++x) {
			const world::TileType &t = assets.data.tile_types[p.TileAt(0, x, y).type];
			for (auto yield : t.resources) {
				yields[yield.resource] += yield.ubiquity;
			}
		}
	}
	int liquid = -1;
	int solid = -1;
	for (auto e : yields) {
		if (assets.data.resources[e.first].state == world::Resource::LIQUID) {
			if (liquid < 0 || e.second > yields[liquid]) {
				liquid = e.first;
			}
		} else if (assets.data.resources[e.first].state == world::Resource::SOLID) {
			if (solid < 0 || e.second > yields[solid]) {
				solid = e.first;
			}
		}
	}

	if (p.HasAtmosphere()) {
		std::cout << "require breathing " << assets.data.resources[p.Atmosphere()].label << std::endl;
		std::unique_ptr<Need> need(new InhaleNeed(p.Atmosphere(), 0.5, 0.1));
		need->name = assets.data.resources[p.Atmosphere()].label;
		need->gain = 0.2;
		need->inconvenient = 0.4;
		need->critical = 0.95;
		c.AddNeed(std::move(need));
	}
	if (liquid > -1) {
		std::cout << "require drinking " << assets.data.resources[liquid].label << std::endl;
		std::unique_ptr<Need> need(new IngestNeed(liquid, 0.2, 0.01));
		need->name = assets.data.resources[liquid].label;
		need->gain = 0.01;
		need->inconvenient = 0.6;
		need->critical = 0.95;
		c.AddNeed(std::move(need));
	}
	if (solid > -1) {
		std::cout << "require eating " << assets.data.resources[solid].label << std::endl;
		std::unique_ptr<Need> need(new IngestNeed(solid, 0.1, 0.001));
		need->name = assets.data.resources[solid].label;
		need->gain = 0.007;
		need->inconvenient = 0.6;
		need->critical = 0.95;
		c.AddNeed(std::move(need));
	}
}

Situation::Situation()
: planet(nullptr)
, position(0.0)
, surface(0)
, type(LOST) {
}

Situation::~Situation() {
}

bool Situation::OnPlanet() const noexcept {
	return type == PLANET_SURFACE;
}

bool Situation::OnSurface() const noexcept {
	return type == PLANET_SURFACE;
}

world::Tile &Situation::GetTile() const noexcept {
	double side_length = planet->SideLength();
	double offset = side_length * 0.5;
	double x = std::max(0.0, std::min(side_length, position.x + offset));
	double y = std::max(0.0, std::min(side_length, position.y + offset));
	return planet->TileAt(surface, int(x), int(y));
}

const world::TileType &Situation::GetTileType() const noexcept {
	return planet->GetSimulation().TileTypes()[GetTile().type];
}

void Situation::SetPlanetSurface(world::Planet &p, int srf, const glm::dvec3 &pos) noexcept {
	type = PLANET_SURFACE;
	planet = &p;
	surface = srf;
	position = pos;
}

}
}
