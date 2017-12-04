#ifndef BLOBS_UI_METER_HPP_
#define BLOBS_UI_METER_HPP_

#include "Widget.hpp"


namespace blobs {
namespace ui {

class Meter
: public Widget {

public:
	Meter();
	~Meter() override;

public:
	Meter *Size(const glm::vec2 &s) noexcept { size = s; BreakParentLayout(); return this; }
	Meter *Padding(const glm::vec2 &p) noexcept { padding = p; BreakParentLayout(); return this; }
	Meter *Border(float b) noexcept { border = b; BreakParentLayout(); return this; }

	Meter *FillColor(const glm::vec4 &c) noexcept { fill_color = c; return this; }
	Meter *BorderColor(const glm::vec4 &c) noexcept { border_color = c; return this; }

	Meter *Value(float v) noexcept { value = v; return this; }

	glm::vec2 Size() override;
	void Draw(app::Assets &, graphics::Viewport &) noexcept override;

private:
	glm::vec4 fill_color;
	glm::vec4 border_color;

	glm::vec2 size;
	glm::vec2 padding;
	float border;

	float value;

};

}
}

#endif
