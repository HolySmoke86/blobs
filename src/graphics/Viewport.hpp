#ifndef BLOBS_GRAPHICS_VIEWPORT_HPP_
#define BLOBS_GRAPHICS_VIEWPORT_HPP_

#include "../math/geometry.hpp"


namespace blobs {
namespace graphics {

class Viewport {

public:
	Viewport(int width, int height);
	~Viewport();

	Viewport(const Viewport &) = delete;
	Viewport &operator =(const Viewport &) = delete;

	Viewport(Viewport &&) = delete;
	Viewport &operator =(Viewport &&) = delete;

public:
	int Width() const {
		return width;
	}
	int Height() const {
		return height;
	}
	void Resize(int w, int h);

	math::Ray ShootPixel(int x, int y) const noexcept {
		return math::Ray({
			((double(x) / double(width)) * 2.0) - 1.0,
			1.0 - ((double(y) / double(height)) * 2.0),
			-1.0 }, { 0.0, 0.0, 1.0 });
	}

	void Clear();
	void ClearDepth();

private:
	int width;
	int height;

};

}
}

#endif
