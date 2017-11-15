#include "Creature.hpp"

#include "Body.hpp"

#include <glm/gtx/transform.hpp>


namespace blobs {
namespace world {

Creature::Creature()
: vao() {
}

Creature::~Creature() {
}


glm::dmat4 Creature::LocalTransform() noexcept {
	// TODO: surface transform
	constexpr double half_height = 0.25;
	return glm::translate(glm::dvec3(position.x, position.y, position.z + body->Radius() + half_height))
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

}
}
