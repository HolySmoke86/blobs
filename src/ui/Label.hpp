#ifndef BLOBS_UI_LABEL_HPP_
#define BLOBS_UI_LABEL_HPP_

#include "align.hpp"
#include "../graphics/Texture.hpp"

#include <string>


namespace blobs {
namespace app {
	struct Assets;
}
namespace graphics {
	class Font;
	class Viewport;
}
namespace ui {

class Label {

public:
	explicit Label(const graphics::Font &);
	~Label();

	Label(const Label &) = delete;
	Label &operator =(const Label &) = delete;

	Label(Label &&) = delete;
	Label &operator =(Label &&) = delete;

public:
	Label &Text(const std::string &);
	Label &Font(const graphics::Font &);
	Label &Foreground(const glm::vec4 &);
	Label &Background(const glm::vec4 &);

	Label &Position(const glm::vec3 &p) noexcept { pos = p; return *this; }
	const glm::vec3 &Position() const noexcept { return pos; }

	Label &Origin(Gravity o) noexcept { origin = o; return *this; }
	Gravity Origin() const noexcept { return origin; }

	glm::vec2 Size();
	void Draw(app::Assets &, graphics::Viewport &) noexcept;

private:
	void Update();

private:
	const graphics::Font *font;
	std::string text;
	graphics::Texture tex;
	glm::vec4 fg_color;
	glm::vec4 bg_color;
	glm::vec3 pos;
	Gravity origin;
	bool dirty;

};

}
}

#endif
