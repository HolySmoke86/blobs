#ifndef BLOBS_APP_INIT_HPP_
#define BLOBS_APP_INIT_HPP_

#include "../graphics/Viewport.hpp"

#include <al.h>
#include <SDL.h>
#include <string>


namespace blobs {
namespace app {

class InitSDL {

public:
	InitSDL();
	~InitSDL();

	InitSDL(const InitSDL &) = delete;
	InitSDL &operator =(const InitSDL &) = delete;

};


class InitVideo {

public:
	InitVideo();
	~InitVideo();

	InitVideo(const InitVideo &) = delete;
	InitVideo &operator =(const InitVideo &) = delete;

};


class InitIMG {

public:
	InitIMG();
	~InitIMG();

	InitIMG(const InitIMG &) = delete;
	InitIMG &operator =(const InitIMG &) = delete;

};


class InitNet {

public:
	InitNet();
	~InitNet();

	InitNet(const InitNet &) = delete;
	InitNet &operator =(const InitNet &) = delete;

};


class InitTTF {

public:
	InitTTF();
	~InitTTF();

	InitTTF(const InitTTF &) = delete;
	InitTTF &operator =(const InitTTF &) = delete;

};


class InitAL {

public:
	InitAL();
	~InitAL();

	InitAL(const InitAL &) = delete;
	InitAL &operator =(const InitAL &) = delete;

};


class InitGL {

public:
	explicit InitGL(bool double_buffer = true, int sample_size = 1);

	InitGL(const InitGL &) = delete;
	InitGL &operator =(const InitGL &) = delete;

};


class Window {

public:
	Window(int w, int h);
	~Window();

	Window(const Window &) = delete;
	Window &operator =(const Window &) = delete;

	void GrabInput();
	void ReleaseInput();

	void GrabMouse();
	void ReleaseMouse();

	SDL_Window *Handle() { return handle; }

	void Flip();

private:
	SDL_Window *handle;

};


class GLContext {

public:
	explicit GLContext(SDL_Window *);
	~GLContext();

	GLContext(const GLContext &) = delete;
	GLContext &operator =(const GLContext &) = delete;

private:
	SDL_GLContext ctx;

};


class InitGLEW {

public:
	InitGLEW();

	InitGLEW(const InitGLEW &) = delete;
	InitGLEW &operator =(const InitGLEW &) = delete;

};


struct Init {

	Init(bool double_buffer = true, int sample_size = 1);

	InitVideo init_video;
	InitIMG init_img;
	InitTTF init_ttf;
	InitAL init_al;
	InitGL init_gl;
	Window window;
	GLContext ctx;
	InitGLEW init_glew;
	graphics::Viewport viewport;

};

}
}

#endif
