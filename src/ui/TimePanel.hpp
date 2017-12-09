#ifndef BLOBS_UI_TIMEPANEL_HPP_
#define BLOBS_UI_TIMEPANEL_HPP_

#include "Panel.hpp"


namespace blobs {
namespace graphics {
	class Viewport;
}
namespace world {
	class Body;
	class Simulation;
}
namespace ui {

class Label;

class TimePanel {

public:
	explicit TimePanel(world::Simulation &);
	~TimePanel();

public:
	void SetBody(world::Body &b) noexcept { body = &b; }
	void UnsetBody() noexcept { body = nullptr; }
	void Draw(graphics::Viewport &) noexcept;

	void ZIndex(float z) noexcept { panel.ZIndex(z); }

private:
	world::Simulation &sim;
	world::Body *body;
	Label *live;
	Label *time;
	Label *clock;
	Panel panel;

};

}
}

#endif
