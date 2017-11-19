#ifndef BLOBS_APP_MASTERSTATE_HPP_
#define BLOBS_APP_MASTERSTATE_HPP_

#include "State.hpp"

#include "Assets.hpp"
#include "../graphics/Camera.hpp"
#include "../ui/CreaturePanel.hpp"


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
	graphics::Camera &GetCamera() noexcept { return cam; }
	const graphics::Camera &GetCamera() const noexcept { return cam; }

	ui::CreaturePanel &GetCreaturePanel() noexcept { return cp; }
	const ui::CreaturePanel &GetCreaturePanel() const noexcept { return cp; }

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

	graphics::Camera cam;
	ui::CreaturePanel cp;

	int remain;
	int thirds;
	bool paused;

};

}
}

#endif
