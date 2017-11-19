#ifndef BLOBS_GRAPHICS_VIEWPORT_HPP_
#define BLOBS_GRAPHICS_VIEWPORT_HPP_


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

	void Clear();
	void ClearDepth();

private:
	int width;
	int height;

};

}
}

#endif
