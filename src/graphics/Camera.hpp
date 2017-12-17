#ifndef BLOBS_GRAPHICS_CAMERA_HPP_
#define BLOBS_GRAPHICS_CAMERA_HPP_

#include "../math/glm.hpp"


namespace blobs {
namespace world {
	class Body;
}
namespace graphics {

class Camera {

public:
	explicit Camera(const world::Body &) noexcept;
	~Camera() noexcept;

	Camera(const Camera &) = delete;
	Camera &operator =(const Camera &) = delete;

	Camera(Camera &&) = delete;
	Camera &operator =(Camera &&) = delete;

public:
	Camera &FOV(float f) noexcept;
	Camera &Aspect(float r) noexcept;
	Camera &Aspect(float w, float h) noexcept;
	Camera &Clip(float near, float far) noexcept;

	const world::Body &Reference() const noexcept { return *ref; }
	Camera &Reference(const world::Body &) noexcept;

	Camera &LookAt(const glm::vec3 &pos, const glm::vec3 &tgt, const glm::vec3 &up) noexcept;

	/// track orientation of reference body
	void TrackOrientation(bool b = true) noexcept { track_orient = b; }

	const glm::mat4 &Projection() const noexcept { return projection; }
	const glm::mat4 &View() const noexcept { return view; }
	glm::mat4 Model(const world::Body &) const noexcept;
	glm::mat4 Universe() const noexcept;

private:
	void UpdateProjection() noexcept;

private:
	float fov;
	float aspect;
	float near;
	float far;

	glm::mat4 projection;
	glm::mat4 view;

	// reference frame
	const world::Body *ref;
	// track reference body's orientation
	bool track_orient;

};

}
}

#endif
