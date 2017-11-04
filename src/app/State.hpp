#ifndef BLOBS_APP_STATE_HPP_
#define BLOBS_APP_STATE_HPP_

#include <SDL.h>


namespace blobs {
namespace app {

class Application;

class State {

	friend class Application;

	void Handle(const SDL_Event &);
	void Handle(const SDL_WindowEvent &);
	void Update(int dt);
	void Render();

	virtual void OnEnter() { }
	virtual void OnResume() { }
	virtual void OnPause() { }
	virtual void OnExit() { }

	virtual void OnFocus() { }
	virtual void OnBlur() { }
	virtual void OnResize() { }

	virtual void OnEvent(const SDL_Event &);
	virtual void OnUpdate(int dt);
	virtual void OnRender();

	int ref_count = 0;

};

}
}

#endif
