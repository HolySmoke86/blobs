#ifndef BLOBS_GRAPHICS_FORMAT_HPP_
#define BLOBS_GRAPHICS_FORMAT_HPP_

#include <SDL.h>
#include <GL/glew.h>


namespace blobs {
namespace graphics {

struct Format {

	GLenum format;
	GLenum type;
	GLenum internal;

	SDL_PixelFormat sdl_format;

	Format() noexcept;
	explicit Format(const SDL_PixelFormat &) noexcept;

	bool Compatible(const Format &other) const noexcept;

};

}
}

#endif
