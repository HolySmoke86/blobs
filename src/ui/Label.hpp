#ifndef BLOBS_UI_LABEL_HPP_
#define BLOBS_UI_LABEL_HPP_

#include "Widget.hpp"
#include "../graphics/Texture.hpp"

#include <string>


namespace blobs {
namespace graphics {
	class Font;
}
namespace ui {

class Label
: public Widget {

public:
	explicit Label(const graphics::Font &);
	~Label() override;

public:
	Label *Text(const std::string &);
	Label *Font(const graphics::Font &);
	Label *Foreground(const glm::vec4 &);
	Label *Background(const glm::vec4 &);

	glm::vec2 Size() override;
	void Draw(app::Assets &, graphics::Viewport &) noexcept override;

private:
	void FixLayout() override;

private:
	const graphics::Font *font;
	std::string text;
	graphics::Texture tex;
	glm::vec4 fg_color;
	glm::vec4 bg_color;

};

}
}

#endif
