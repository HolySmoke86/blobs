#ifndef BLOBS_UI_WIDGET_HPP_
#define BLOBS_UI_WIDGET_HPP_

#include "align.hpp"
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
	Widget &Position(const glm::vec3 &p) noexcept { pos = p; return *this; }
	const glm::vec3 &Position() const noexcept { return pos; }
	glm::vec3 AlignedPosition() noexcept;
	glm::vec3 TopLeft() noexcept;

	Widget &Origin(Gravity o) noexcept { origin = o; return *this; }
	Gravity Origin() const noexcept { return origin; }

	virtual glm::vec2 Size() = 0;
	virtual void Draw(app::Assets &, graphics::Viewport &) noexcept = 0;

private:
	glm::vec3 pos;
	Gravity origin;

};

}
}

#endif
