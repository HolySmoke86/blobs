#include "Camera.hpp"
#include "Viewport.hpp"

#include "../creature/Creature.hpp"
#include "../math/const.hpp"
#include "../world/Body.hpp"
#include "../world/Planet.hpp"

#include <cmath>
#include <GL/glew.h>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/rotate_vector.hpp>
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

Camera &Camera::Orbital(const glm::vec3 &pos) noexcept {
	track_orient = false;
	view = glm::lookAt(pos, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	return *this;
}

Camera &Camera::Radial(const creature::Creature &c, double distance, const glm::dvec3 &angle) {
	const creature::Situation &s = c.GetSituation();
	glm::dvec3 pos(s.Position());
	glm::dvec3 up(0.0);
	glm::dvec3 dir(0.0, 0.0, -distance);
	if (s.OnSurface()) {
		Reference(s.GetPlanet());
		track_orient = true;
		up = s.GetPlanet().NormalAt(s.Position());
		glm::dvec3 ref(normalize(cross(up, glm::dvec3(up.z, up.x, up.y))));
		dir =
			glm::dmat3(ref, up, cross(ref, up))
			* glm::dmat3(glm::eulerAngleYX(-angle.y, -angle.x))
			* dir;
	} else {
		up.y = 1.0;
		dir = glm::dmat3(glm::eulerAngleYX(-angle.y, -angle.x)) * dir;
	}
	pos += up * (c.Size() * 0.5);
	up = glm::rotate(up, angle.z, glm::normalize(-dir));
	view = glm::lookAt(pos - dir, pos, up);
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

void Viewport::ClearDepth() {
	glClear(GL_DEPTH_BUFFER_BIT);
}

}
}
