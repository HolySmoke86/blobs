#include "Camera.hpp"
#include "Viewport.hpp"

#include "../const.hpp"
#include "../world/Body.hpp"

#include <cmath>
#include <GL/glew.h>
#include <glm/gtx/transform.hpp>


namespace blobs {
namespace graphics {

Camera::Camera(const world::Body &r) noexcept
: fov(PI_0p25)
, aspect(1.0f)
, near(0.1f)
, far(12560.0f)
, projection(glm::perspective(fov, aspect, near, far))
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

Camera &Camera::FirstPerson(int srf, const glm::vec3 &pos, const glm::vec3 &at) noexcept {
	track_orient = true;

	float dir = srf < 3 ? 1.0f : -1.0f;

	glm::vec3 position;
	position[(srf + 0) % 3] = pos.x;
	position[(srf + 1) % 3] = pos.y;
	position[(srf + 2) % 3] = dir * (pos.z + Reference().Radius());

	glm::vec3 up(0.0f);
	up[(srf + 2) % 3] = dir;

	glm::vec3 target;
	target[(srf + 0) % 3] = at.x;
	target[(srf + 1) % 3] = at.y;
	target[(srf + 2) % 3] = dir * (at.z + Reference().Radius());

	view = glm::lookAt(position, target, up);

	return *this;
}

Camera &Camera::MapView(int srf, const glm::vec3 &pos, float roll) noexcept {
	track_orient = true;

	float dir = srf < 3 ? 1.0f : -1.0f;

	glm::vec3 position;
	position[(srf + 0) % 3] = pos.x;
	position[(srf + 1) % 3] = pos.y;
	position[(srf + 2) % 3] = dir * (pos.z + Reference().Radius());

	glm::vec3 up(0.0f);
	up[(srf + 0) % 3] = std::cos(roll);
	up[(srf + 1) % 3] = std::sin(roll);
	up[(srf + 2) % 3] = 0.0f;

	glm::vec3 target = position;
	target[(srf + 2) % 3] -= dir;

	view = glm::lookAt(position, target, up);

	return *this;
}

Camera &Camera::Orbital(const glm::vec3 &pos) noexcept {
	track_orient = false;
	view = glm::lookAt(pos, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	return *this;
}

glm::mat4 Camera::Model(const world::Body &b) const noexcept {
	if (&b == ref) {
		return track_orient ? glm::mat4(1.0f) : glm::mat4(ref->LocalTransform());
	} else if (b.HasParent() && &b.Parent() == ref) {
		return track_orient
			? ref->InverseTransform() * b.FromParent() * b.LocalTransform()
			: b.FromParent() * b.LocalTransform();
	} else if (ref->HasParent() && &ref->Parent() == &b) {
		return track_orient
			? ref->InverseTransform() * ref->ToParent() * b.LocalTransform()
			: ref->ToParent() * b.LocalTransform();
	} else {
		return track_orient
			? ref->InverseTransform() * ref->ToUniverse() * b.FromUniverse() * b.LocalTransform()
			: ref->ToUniverse() * b.FromUniverse() * b.LocalTransform();
	}
}

void Camera::UpdateProjection() noexcept {
	projection = glm::perspective(fov, aspect, near, far);
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

}
}
