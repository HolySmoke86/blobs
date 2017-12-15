#include "Camera.hpp"
#include "Viewport.hpp"

#include "../creature/Creature.hpp"
#include "../math/const.hpp"
#include "../world/Body.hpp"
#include "../world/Planet.hpp"

#include <cmath>
#include <GL/glew.h>
#include <glm/gtx/transform.hpp>


namespace blobs {
namespace graphics {

Camera::Camera(const world::Body &r) noexcept
: fov(PI * 0.25)
, aspect(1.0f)
, near(0.1f)
, far(12560.0f)
, projection(glm::infinitePerspective(fov, aspect, near))
, view(1.0f)
, ref(&r)
, track_orient(false) {

}

Camera::~Camera() noexcept {
}

Camera &Camera::FOV(float f) noexcept {
	fov = f;
	UpdateProjection();
	return *this;
}

Camera &Camera::Aspect(float r) noexcept {
	aspect = r;
	UpdateProjection();
	return *this;
}

Camera &Camera::Aspect(float w, float h) noexcept {
	Aspect(w / h);
	return *this;
}

Camera &Camera::Clip(float n, float f) noexcept {
	near = n;
	far = f;
	UpdateProjection();
	return *this;
}

Camera &Camera::Reference(const world::Body &r) noexcept {
	ref = &r;
	return *this;
}

Camera &Camera::LookAt(const glm::vec3 &pos, const glm::vec3 &tgt, const glm::vec3 &up) noexcept {
	view = glm::lookAt(pos, tgt, up);
	return *this;
}

glm::mat4 Camera::Model(const world::Body &b) const noexcept {
	if (&b == ref) {
		return track_orient ? glm::mat4(1.0f) : glm::mat4(ref->LocalTransform());
	} else if (b.HasParent() && &b.Parent() == ref) {
		return glm::mat4(track_orient
			? ref->InverseTransform() * b.FromParent() * b.LocalTransform()
			: b.FromParent() * b.LocalTransform());
	} else if (ref->HasParent() && &ref->Parent() == &b) {
		return glm::mat4(track_orient
			? ref->InverseTransform() * ref->ToParent() * b.LocalTransform()
			: ref->ToParent() * b.LocalTransform());
	} else {
		return glm::mat4(track_orient
			? ref->InverseTransform() * ref->ToUniverse() * b.FromUniverse() * b.LocalTransform()
			: ref->ToUniverse() * b.FromUniverse() * b.LocalTransform());
	}
}

void Camera::UpdateProjection() noexcept {
	projection = glm::infinitePerspective(fov, aspect, near);
}


Viewport::Viewport(int w, int h)
: width(w)
, height(h) {
	Resize(w, h);
	glClearColor(0.0, 0.0, 0.0, 1.0);
}

Viewport::~Viewport() {
}


void Viewport::Resize(int w, int h) {
	width = w;
	height = h;
	glViewport(0, 0, w, h);
}

void Viewport::Clear() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Viewport::ClearDepth() {
	glClear(GL_DEPTH_BUFFER_BIT);
}

}
}
