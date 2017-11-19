#include "Label.hpp"
#include "Panel.hpp"
#include "Widget.hpp"

#include "../app/Assets.hpp"
#include "../graphics/Font.hpp"
#include "../graphics/Viewport.hpp"

#include <glm/gtx/transform.hpp>


namespace blobs {
namespace ui {

Label::Label(const graphics::Font &f)
: font(&f)
, text()
, tex()
, fg_color(0.0f, 0.0f, 0.0f, 1.0f)
, bg_color(0.0f, 0.0f, 0.0f, 0.0f)
, dirty(true) {
}

Label::~Label() {
}

Label &Label::Text(const std::string &t) {
	if (text != t) {
		dirty = true;
	}
	text = t;
	return *this;
}

Label &Label::Font(const graphics::Font &f) {
	if (font != &f) {
		dirty = true;
	}
	font = &f;
	return *this;
}

Label &Label::Foreground(const glm::vec4 &c) {
	fg_color = c;
	return *this;
}

Label &Label::Background(const glm::vec4 &c) {
	bg_color = c;
	return *this;
}

glm::vec2 Label::Size() {
	if (text.empty()) {
		return glm::vec2(0.0f);
	}
	Update();
	return tex.Size();
}

void Label::Draw(app::Assets &assets, graphics::Viewport &viewport) noexcept {
	Update();
	glm::vec2 size = Size();

	assets.shaders.alpha_sprite.Activate();
	assets.shaders.alpha_sprite.SetM(glm::translate(AlignedPosition())
		* glm::scale(glm::vec3(size.x, size.y, 1.0f)));
	assets.shaders.alpha_sprite.SetTexture(tex);
	assets.shaders.alpha_sprite.SetFgColor(fg_color);
	assets.shaders.alpha_sprite.SetBgColor(bg_color);
	assets.shaders.alpha_sprite.DrawRect();
}

void Label::Update() {
	if (!dirty) return;
	font->Render(text, tex);
	dirty = false;
}


Panel::Panel()
: widgets()
, bg_color(0.0f, 0.0f, 0.0f, 0.0f)
, padding(0.0f)
, spacing(0.0f)
, dir(VERTICAL)
, size(0.0f, 0.0f) {
}

Panel::~Panel() {
}

Panel &Panel::Add(Widget *w) {
	std::unique_ptr<Widget> widget(w);
	glm::vec2 wsize = widget->Size();
	if (dir == HORIZONTAL) {
		size.x += wsize.x;
		size.y = std::max(size.y, wsize.y);
	} else {
		size.x = std::max(size.x, wsize.x);
		size.y += wsize.y;
	}
	widgets.emplace_back(std::move(widget));
	return *this;
}

Panel &Panel::Background(const glm::vec4 &c) {
	bg_color = c;
	return *this;
}

Panel &Panel::Padding(const glm::vec2 &p) {
	padding = p;
	return *this;
}

Panel &Panel::Spacing(float s) {
	spacing = s;
	return *this;
}

Panel &Panel::Direction(Dir d) {
	dir = d;
	Relayout();
	return *this;
}

glm::vec2 Panel::Size() {
	return 2.0f * padding + glm::vec2(0.0f, (widgets.size() - 1) * spacing) + size;
}

void Panel::Relayout() {
	size = glm::vec2(0.0f);
	if (dir == HORIZONTAL) {
		for (auto &w : widgets) {
			glm::vec2 wsize = w->Size();
			size.x += wsize.x;
			size.y = std::max(size.y, wsize.y);
		}
	} else {
		for (auto &w : widgets) {
			glm::vec2 wsize = w->Size();
			size.x = std::max(size.x, wsize.x);
			size.y += wsize.y;
		}
	}
}

void Panel::Draw(app::Assets &assets, graphics::Viewport &viewport) noexcept {
	if (bg_color.a > 0.0f) {
		glm::vec2 fullsize = Size();
		assets.shaders.plain_color.Activate();
		assets.shaders.plain_color.SetM(glm::translate(AlignedPosition())
			* glm::scale(glm::vec3(fullsize.x, fullsize.y, 1.0f)));
		assets.shaders.plain_color.SetColor(bg_color);
		assets.shaders.plain_color.DrawRect();
	}

	glm::vec3 cursor = TopLeft();
	cursor.x += padding.x;
	cursor.y += padding.y;
	cursor.z -= 1.0f;
	for (auto &w : widgets) {
		w->Position(cursor).Origin(Gravity::NORTH_WEST);
		w->Draw(assets, viewport);
		cursor[dir] += w->Size()[dir] + spacing;
	}
}


Widget::Widget()
: pos(0.0f)
, origin(Gravity::CENTER) {
}

Widget::~Widget() {
}

glm::vec3 Widget::AlignedPosition() noexcept {
	return align(origin, Size(), pos);
}

glm::vec3 Widget::TopLeft() noexcept {
	glm::vec2 size = Size();
	return align(origin, size, pos) - glm::vec3(size * 0.5f, 0.0f);
}

}
}
