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

private:
	world::Simulation &sim;
	world::Body *body;
	Label *time;
	Label *clock;
	Panel panel;

};

}
}

#endif
