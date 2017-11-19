#ifndef BLOBS_UI_PANEL_HPP_
#define BLOBS_UI_PANEL_HPP_

#include "Widget.hpp"

#include <memory>
#include <vector>


namespace blobs {
namespace ui {

class Panel
: public Widget {

public:
	enum Dir {
		HORIZONTAL,
		VERTICAL,
	};

public:
	Panel();
	~Panel() override;

public:
	// panel takes ownership
	Panel &Add(Widget *);

	Panel &Background(const glm::vec4 &);
	Panel &Padding(const glm::vec2 &);
	Panel &Spacing(float);
	Panel &Direction(Dir);

	glm::vec2 Size() override;
	void Relayout();
	void Draw(app::Assets &, graphics::Viewport &) noexcept override;

private:
	std::vector<std::unique_ptr<Widget>> widgets;
	glm::vec4 bg_color;
	glm::vec2 padding;
	float spacing;
	Dir dir;
	glm::vec2 size;

};

}
}

#endif
