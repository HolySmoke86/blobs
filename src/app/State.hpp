#ifndef BLOBS_APP_STATE_HPP_
#define BLOBS_APP_STATE_HPP_

#include <SDL.h>


namespace blobs {
namespace graphics {
	class Viewport;
}
namespace app {

class Application;

class State {

	friend class Application;

	void Handle(const SDL_Event &);
	void Handle(const SDL_WindowEvent &);
	void Update(int dt);
	void Render(graphics::Viewport &);

	virtual void OnEnter() { }
	virtual void OnResume() { }
	virtual void OnPause() { }
	virtual void OnExit() { }

	virtual void OnFocus() { }
	virtual void OnBlur() { }
	virtual void OnResize(int w, int h) { }

	virtual void OnKeyDown(const SDL_KeyboardEvent &) { }
	virtual void OnKeyUp(const SDL_KeyboardEvent &) { }
	virtual void OnMouseDown(const SDL_MouseButtonEvent &) { }
	virtual void OnMouseUp(const SDL_MouseButtonEvent &) { }
	virtual void OnMouseMotion(const SDL_MouseMotionEvent &) { }
	virtual void OnMouseWheel(const SDL_MouseWheelEvent &) { }
	virtual void OnQuit();

	virtual void OnUpdate(int dt) { }
	virtual void OnRender(graphics::Viewport &) { }

	int ref_count = 0;
	Application *app = nullptr;

protected:
	Application &App() {
		return *app;
	}

};

}
}

#endif
