#ifndef BLOBS_TEST_EVENT_HPP_
#define BLOBS_TEST_EVENT_HPP_

#include <SDL.h>


namespace blobs {
namespace test {

inline void FakeKeyDown(int sym) {
	SDL_Event e;
	e.type = SDL_KEYDOWN;
	e.key.keysym.sym = sym;
	SDL_PushEvent(&e);
}

inline void FakeKeyUp(int sym) {
	SDL_Event e;
	e.type = SDL_KEYUP;
	e.key.keysym.sym = sym;
	SDL_PushEvent(&e);
}

inline void FakeKeyPress(int sym) {
	FakeKeyDown(sym);
	FakeKeyUp(sym);
}

inline void FakeMouseDown(int button = SDL_BUTTON_LEFT, int x = 0, int y = 0) {
	SDL_Event e;
	e.type = SDL_MOUSEBUTTONDOWN;
	e.button.button = button;
	e.button.x = x;
	e.button.y = y;
	SDL_PushEvent(&e);
}

inline void FakeMouseUp(int button = SDL_BUTTON_LEFT, int x = 0, int y = 0) {
	SDL_Event e;
	e.type = SDL_MOUSEBUTTONUP;
	e.button.button = button;
	e.button.x = x;
	e.button.y = y;
	SDL_PushEvent(&e);
}

inline void FakeMouseClick(int button = SDL_BUTTON_LEFT, int x = 0, int y = 0) {
	FakeMouseDown(button, x, y);
	FakeMouseUp(button, x, y);
}

inline void FakeMouseMotion(int xrel = 0, int yrel = 0) {
	SDL_Event e;
	e.type = SDL_MOUSEMOTION;
	e.motion.xrel = xrel;
	e.motion.yrel = yrel;
	SDL_PushEvent(&e);
}

inline void FakeMouseWheel(int y = 0, int x = 0) {
	SDL_Event e;
	e.type = SDL_MOUSEWHEEL;
	e.wheel.x = x;
	e.wheel.y = y;
	SDL_PushEvent(&e);
}

inline void FakeQuit() {
	SDL_Event e;
	e.type = SDL_QUIT;
	SDL_PushEvent(&e);
}

}
}

#endif
