#include "CreaturePanel.hpp"

#include "../app/Assets.hpp"
#include "../creature/Creature.hpp"
#include "../graphics/Viewport.hpp"

#include <glm/gtx/transform.hpp>

#include <iostream>
#include <glm/gtx/io.hpp>


namespace blobs {
namespace ui {

CreaturePanel::CreaturePanel(const app::Assets &assets)
: c(nullptr)
, name(assets.fonts.large) {
	name.Origin(Gravity::NORTH_EAST);
}

CreaturePanel::~CreaturePanel() {
}


void CreaturePanel::Show(creature::Creature &cr) {
	c = &cr;
	name.Text(c->Name());
}

void CreaturePanel::Hide() noexcept {
	c = nullptr;
}

void CreaturePanel::Draw(app::Assets &assets, graphics::Viewport &viewport) noexcept {
	if (!c) return;

	const glm::vec2 margin(20.0f);
	const glm::vec2 padding(10.0f);

	const glm::vec2 size(name.Size() + 2.0f * padding);
	const glm::vec2 half_size = size * 0.5f;

	const glm::vec3 top_right(viewport.Width() - margin.x, margin.y, 0.0f);
	name.Position(top_right - glm::vec3(padding.x, -padding.y, 1.0f));

	assets.shaders.plain_color.Activate();
	assets.shaders.plain_color.SetM(
		glm::translate(glm::vec3(top_right.x - half_size.x, top_right.y + half_size.y, 0.0f))
		* glm::scale(glm::vec3(half_size.x, half_size.y, 1.0f)));
	assets.shaders.plain_color.SetColor(glm::vec3(0.7f, 0.7f, 0.7f));
	assets.shaders.plain_color.DrawRect();

	name.Draw(assets, viewport);
}


Label::Label(const graphics::Font &f)
: font(&f)
, text()
, tex()
, fg_color(0.0f, 0.0f, 0.0f, 1.0f)
, bg_color(0.0f, 0.0f, 0.0f, 0.0f)
, pos(0.0f, 0.0f, 0.0f)
, origin(Gravity::CENTER)
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
	Update();
	return tex.Size();
}

void Label::Draw(app::Assets &assets, graphics::Viewport &viewport) noexcept {
	Update();
	glm::vec2 size = Size();
	glm::vec3 position = align(origin, size, pos);

	std::cout << "pos: " << pos << ", size: " << size << ", position: " << position << std::endl;

	assets.shaders.alpha_sprite.Activate();
	assets.shaders.alpha_sprite.SetM(glm::translate(position)
		* glm::scale(glm::vec3(size.x * 0.5f, size.y * 0.5f, 1.0f)));
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

}
}
