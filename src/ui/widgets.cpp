#include "Label.hpp"
#include "Meter.hpp"
#include "Panel.hpp"
#include "Widget.hpp"

#include "../app/Assets.hpp"
#include "../graphics/Font.hpp"
#include "../graphics/Viewport.hpp"

#include <iomanip>
#include <sstream>
#include <glm/gtx/transform.hpp>


namespace blobs {
namespace ui {

Label::Label(const graphics::Font &f)
: font(&f)
, text()
, tex()
, fg_color(0.0f, 0.0f, 0.0f, 1.0f)
, bg_color(0.0f, 0.0f, 0.0f, 0.0f) {
}

Label::~Label() {
}

Label *Label::Text(const std::string &t) {
	if (text != t) {
		BreakLayout();
	}
	text = t;
	return this;
}

Label *Label::Font(const graphics::Font &f) {
	if (font != &f) {
		BreakLayout();
	}
	font = &f;
	return this;
}

Label *Label::Foreground(const glm::vec4 &c) {
	fg_color = c;
	return this;
}

Label *Label::Background(const glm::vec4 &c) {
	bg_color = c;
	return this;
}

glm::vec2 Label::Size() {
	if (text.empty()) {
		return glm::vec2(0.0f);
	}
	return tex.Size();
}

void Label::Draw(app::Assets &assets, graphics::Viewport &viewport) noexcept {
	if (text.empty()) return;
	glm::vec2 size = Size();

	assets.shaders.alpha_sprite.Activate();
	assets.shaders.alpha_sprite.SetM(glm::translate(glm::vec3(Position() + (size * 0.5f), -ZIndex()))
		* glm::scale(glm::vec3(size.x, size.y, 1.0f)));
	assets.shaders.alpha_sprite.SetTexture(tex);
	assets.shaders.alpha_sprite.SetFgColor(fg_color);
	assets.shaders.alpha_sprite.SetBgColor(bg_color);
	assets.shaders.alpha_sprite.DrawRect();
}

void Label::FixLayout() {
	if (text.empty()) return;
	font->Render(text, tex);
}


Meter::Meter()
: fill_color(1.0f)
, border_color(1.0f)
, size(3.0f)
, padding(1.0f)
, border(1.0f)
, value(0.0f) {
}

Meter::~Meter() {
}

glm::vec2 Meter::Size() {
	return size + (2.0f * padding) + glm::vec2(2.0f * border);
}

void Meter::Draw(app::Assets &assets, graphics::Viewport &viewport) noexcept {
	glm::vec2 fullsize = Size();
	assets.shaders.canvas.Activate();
	assets.shaders.canvas.ZIndex(ZIndex());

	if (border > 0.0f) {
		assets.shaders.canvas.SetColor(border_color);
		assets.shaders.canvas.DrawRect(
			Position() + glm::vec2(border * 0.5f),
			Position() + fullsize - glm::vec2(border * 0.5f),
			border
		);
	}

	if (value > 0.0f) {
		glm::vec2 bottom_right = Position() + fullsize - glm::vec2(border) - padding;
		bottom_right.x -= size.x * (1.0f - value);
		assets.shaders.canvas.SetColor(fill_color);
		assets.shaders.canvas.FillRect(
			Position() + glm::vec2(border) + padding,
			bottom_right
		);
	}
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

Panel *Panel::Add(Widget *w) {
	w->SetParent(*this);
	widgets.emplace_back(std::unique_ptr<Widget>(w));
	BreakLayout();
	return this;
}

Panel *Panel::Clear() {
	widgets.clear();
	BreakLayout();
	return this;
}

Panel *Panel::Background(const glm::vec4 &c) {
	bg_color = c;
	return this;
}

Panel *Panel::Padding(const glm::vec2 &p) {
	padding = p;
	BreakParentLayout();
	return this;
}

Panel *Panel::Spacing(float s) {
	spacing = s;
	BreakParentLayout();
	return this;
}

Panel *Panel::Direction(Dir d) {
	dir = d;
	BreakLayout();
	return this;
}

glm::vec2 Panel::Size() {
	glm::vec2 space(0.0f);
	space[dir] = (widgets.size() - 1) * spacing;
	return (2.0f * padding) + space + size;
}

void Panel::FixLayout() {
	for (auto &w : widgets) {
		w->Layout();
	}
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
		assets.shaders.canvas.Activate();
		assets.shaders.canvas.ZIndex(ZIndex());
		assets.shaders.canvas.SetColor(bg_color);
		assets.shaders.canvas.FillRect(Position(), Position() + Size());
	}

	glm::vec2 cursor = Position() + padding;
	for (auto &w : widgets) {
		w->Position(cursor)->ZIndex(ZIndex() + 1.0f);
		w->Draw(assets, viewport);
		cursor[dir] += w->Size()[dir] + spacing;
	}
}


Widget::Widget()
: parent(nullptr)
, pos(0.0f)
, z_index(1.0f)
, dirty_layout(false) {
}

Widget::~Widget() {
}

void Widget::SetParent(Widget &p) noexcept {
	parent = &p;
}

void Widget::BreakLayout() noexcept {
	if (dirty_layout) return;
	dirty_layout = true;
	BreakParentLayout();
}

void Widget::BreakParentLayout() noexcept {
	if (HasParent()) {
		GetParent().BreakLayout();
	}
}

void Widget::Layout() {
	if (dirty_layout) {
		FixLayout();
		dirty_layout = false;
	}
}

}
}
