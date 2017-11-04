#ifndef BLOBS_APP_APPLICATION_HPP_
#define BLOBS_APP_APPLICATION_HPP_

#include <stack>


namespace blobs {
namespace app {

class State;

class Application {

public:
	Application();
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

	/// Loop until states is empty.
	void Run();
	/// Evaluate a single frame of dt milliseconds.
	void Loop(int dt);
	/// Process all events in queue.
	void HandleEvents();

private:
	std::stack<State *> states;

};

}
}

#endif
