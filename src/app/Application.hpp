#ifndef BLOBS_APP_APPLICATION_HPP_
#define BLOBS_APP_APPLICATION_HPP_

#include <stack>


namespace blobs {
namespace graphics {
	class Viewport;
}
namespace app {

class State;
class Window;

class Application {

public:
	Application(Window &, graphics::Viewport &);
	~Application();

	Application(const Application &) = delete;
	Application &operator =(const Application &) = delete;

	Application(Application &&) = delete;
	Application &operator =(Application &&) = delete;

public:
	void PushState(State *);
	State *PopState();
	State *SwitchState(State *);
	State &GetState();
	bool HasState() const noexcept;

	Window &GetWindow() noexcept { return window; }
	const Window &GetWindow() const noexcept { return window; }

	graphics::Viewport &GetViewport() noexcept { return viewport; }
	const graphics::Viewport &GetViewport() const noexcept { return viewport; }

	/// Loop until states is empty.
	void Run();
	/// Evaluate a single frame of dt milliseconds.
	void Loop(int dt);
	/// Process all events in queue.
	void HandleEvents();

private:
	Window &window;
	graphics::Viewport &viewport;
	std::stack<State *> states;

};

}
}

#endif
