#ifndef BLOBS_UI_WIDGET_HPP_
#define BLOBS_UI_WIDGET_HPP_

#include "../graphics/glm.hpp"


namespace blobs {
namespace app {
	struct Assets;
}
namespace graphics {
	class Viewport;
}
namespace ui {

class Widget {

public:
	Widget();
	virtual ~Widget();

	Widget(const Widget &) = delete;
	Widget &operator =(const Widget &) = delete;

	Widget(Widget &&) = delete;
	Widget &operator =(Widget &&) = delete;

public:
	Widget *Position(const glm::vec2 &p) noexcept { pos = p; return this; }
	const glm::vec2 &Position() const noexcept { return pos; }

	Widget *ZIndex(float z) noexcept { z_index = z; return this; }
	float ZIndex() const noexcept { return z_index; }

	virtual glm::vec2 Size() = 0;
	virtual void Draw(app::Assets &, graphics::Viewport &) noexcept = 0;

private:
	glm::vec2 pos;
	float z_index;

};

}
}

#endif
