#ifndef BLOBS_APP_MASTERSTATE_HPP_
#define BLOBS_APP_MASTERSTATE_HPP_

#include "State.hpp"

#include "Assets.hpp"
#include "../graphics/Camera.hpp"
#include "../ui/CreaturePanel.hpp"
#include "../ui/RecordsPanel.hpp"
#include "../ui/TimePanel.hpp"


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

	ui::RecordsPanel &GetRecordsPanel() noexcept { return rp; }
	const ui::RecordsPanel &GetRecordsPanel() const noexcept { return rp; }

	ui::TimePanel &GetTimePanel() noexcept { return tp; }
	const ui::TimePanel &GetTimePanel() const noexcept { return tp; }

private:
	void OnResize(int w, int h) override;

	void OnKeyDown(const SDL_KeyboardEvent &) override;
	void OnMouseDown(const SDL_MouseButtonEvent &) override;
	void OnMouseUp(const SDL_MouseButtonEvent &) override;
	void OnMouseMotion(const SDL_MouseMotionEvent &) override;
	void OnMouseWheel(const SDL_MouseWheelEvent &) override;

	void OnUpdate(int dt) override;
	void OnRender(graphics::Viewport &) override;

	void Tick();
	int FrameMS() const noexcept;

private:
	Assets &assets;
	world::Simulation &sim;

	graphics::Camera cam;
	double cam_dist;
	double cam_tgt_dist;
	glm::dvec3 cam_orient;
	bool cam_dragging;

	ui::CreaturePanel cp;
	ui::RecordsPanel rp;
	ui::TimePanel tp;

	int remain;
	int thirds;
	bool paused;

};

}
}

#endif
