#include "Camera.hpp"
#include "Viewport.hpp"

#include "const.hpp"

#include <GL/glew.h>
#include <glm/gtx/transform.hpp>


namespace blobs {
namespace graphics {

Camera::Camera() noexcept
: fov(PI_0p25)
, aspect(1.0f)
, near(0.1f)
, far(256.0f)
, projection(glm::perspective(fov, aspect, near, far))
, view(1.0f) {

}

Camera::~Camera() noexcept {
}

void Camera::FOV(float f) noexcept {
	fov = f;
	UpdateProjection();
}

void Camera::Aspect(float r) noexcept {
	aspect = r;
	UpdateProjection();
}

void Camera::Aspect(float w, float h) noexcept {
	Aspect(w / h);
}

void Camera::Clip(float n, float f) noexcept {
	near = n;
	far = f;
	UpdateProjection();
}

void Camera::View(const glm::mat4 &v) noexcept {
	view = v;
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
