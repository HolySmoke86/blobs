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

	void Show() noexcept { shown = true; }
	void Hide() noexcept { shown = false; }
	void Toggle() noexcept { shown = !shown; }
	bool Shown() const noexcept { return shown; }

	void ZIndex(float z) noexcept { panel.ZIndex(z); }

private:
	world::Simulation &sim;
	std::vector<Label *> records;
	std::vector<Label *> holders;
	Panel panel;
	bool shown;

};

}
}

#endif
