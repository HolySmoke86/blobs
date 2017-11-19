#include "CreaturePanel.hpp"

#include "../app/Assets.hpp"
#include "../creature/Creature.hpp"
#include "../graphics/Viewport.hpp"

#include <glm/gtx/transform.hpp>


namespace blobs {
namespace ui {

CreaturePanel::CreaturePanel(const app::Assets &assets)
: c(nullptr)
, name(new Label(assets.fonts.large))
, panel() {
	panel
		.Add(name)
		.Padding(glm::vec2(10.0f))
		.Spacing(10.0f)
		.Direction(Panel::VERTICAL)
		.Background(glm::vec4(0.7f, 0.7f, 0.7f, 1.0f))
		.Origin(Gravity::NORTH_EAST);
}

CreaturePanel::~CreaturePanel() {
}


void CreaturePanel::Show(creature::Creature &cr) {
	c = &cr;
	name->Text(c->Name());
	panel.Relayout();
}

void CreaturePanel::Hide() noexcept {
	c = nullptr;
}

void CreaturePanel::Draw(app::Assets &assets, graphics::Viewport &viewport) noexcept {
	if (!c) return;

	const glm::vec2 margin(20.0f);

	panel.Position(glm::vec3(viewport.Width() - margin.x, margin.y, 0.0f));
	panel.Draw(assets, viewport);
}

}
}
