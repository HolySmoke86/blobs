#ifndef BLOBS_GRAPHICS_BUFFER_HPP_
#define BLOBS_GRAPHICS_BUFFER_HPP_

#include "../app/error.hpp"

#include <algorithm>
#include <GL/glew.h>


namespace blobs {
namespace graphics {

template<class T>
class MappedBuffer {

public:
	MappedBuffer(GLenum target, GLenum access)
	: buf(reinterpret_cast<T *>(glMapBuffer(target, access)))
	, target(target) {
		if (!buf) {
			throw app::GLError("failed to map buffer");
		}
	}
	MappedBuffer()
	: buf(nullptr)
	, target(0) {
	}
	~MappedBuffer() noexcept {
		if (buf) {
			glUnmapBuffer(target);
		}
	}

	MappedBuffer(MappedBuffer<T> &&other) noexcept
	: buf(other.buf)
	, target(other.target) {
		other.buf = nullptr;
	}
	MappedBuffer<T> &operator =(MappedBuffer<T> &&other) noexcept {
		std::swap(buf, other.buf);
		std::swap(target, other.target);
	}

	MappedBuffer(const MappedBuffer<T> &) = delete;
	MappedBuffer<T> &operator =(const MappedBuffer<T> &) = delete;

	explicit operator bool() const noexcept { return buf; }

public:
	T &operator [](std::size_t i) noexcept { return buf[i]; }
	const T &operator [](std::size_t i) const noexcept { return buf[i]; }

private:
	T *buf;
	GLenum target;

};

}
}

#endif
