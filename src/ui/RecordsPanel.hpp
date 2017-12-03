#ifndef BLOBS_UI_RECORDSPANEL_HPP_
#define BLOBS_UI_RECORDSPANEL_HPP_

#include "Panel.hpp"


namespace blobs {
namespace graphics {
	class Viewport;
}
namespace world {
	class Simulation;
}
namespace ui {

class Label;

class RecordsPanel {

public:
	explicit RecordsPanel(world::Simulation &);
	~RecordsPanel();

public:
	void Draw(graphics::Viewport &) noexcept;

private:
	world::Simulation &sim;
	Label *live;
	std::vector<Label *> records;
	std::vector<Label *> holders;
	Panel panel;

};

}
}

#endif
