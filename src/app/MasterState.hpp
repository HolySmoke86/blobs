#ifndef BLOBS_APP_MASTERSTATE_HPP_
#define BLOBS_APP_MASTERSTATE_HPP_

#include "State.hpp"

#include "Assets.hpp"
#include "../graphics/Camera.hpp"


namespace blobs {
namespace world {
	class Body;
	class Simulation;
}
namespace app {

class MasterState
: public State {

public:
	MasterState(Assets &, world::Simulation &) noexcept;
	~MasterState() noexcept;

	MasterState(const MasterState &) = delete;
	MasterState &operator =(const MasterState &) = delete;

	MasterState(MasterState &&) = delete;
	MasterState &operator =(MasterState &&) = delete;

public:
	void SetReference(world::Body &r) { reference = &r; }

private:
	void OnResize(int w, int h) override;

	void OnKeyDown(const SDL_KeyboardEvent &) override;

	void OnUpdate(int dt) override;
	void OnRender(graphics::Viewport &) override;

	void Tick();
	int FrameMS() const noexcept;

private:
	Assets &assets;
	world::Simulation &sim;
	world::Body *reference;

	graphics::Camera cam;

	int remain;
	int thirds;
	bool paused;

};

}
}

#endif
