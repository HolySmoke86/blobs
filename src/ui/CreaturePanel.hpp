#ifndef BLOBS_UI_CREATUREPANEL_HPP_
#define BLOBS_UI_CREATUREPANEL_HPP_

#include "Panel.hpp"


namespace blobs {
namespace app {
	struct Assets;
}
namespace creature {
	class Creature;
}
namespace graphics {
	class Viewport;
}
namespace ui {

class Label;
class Meter;

class CreaturePanel {

public:
	explicit CreaturePanel(const app::Assets &);
	~CreaturePanel();

	CreaturePanel(const CreaturePanel &) = delete;
	CreaturePanel &operator =(const CreaturePanel &) = delete;

	CreaturePanel(CreaturePanel &&) = delete;
	CreaturePanel &operator =(CreaturePanel &&) = delete;

public:
	void Show(creature::Creature &);
	void Hide() noexcept;

	void Draw(app::Assets &, graphics::Viewport &) noexcept;

private:
	void CreateNeeds();

private:
	const app::Assets &assets;
	creature::Creature *c;

	Label *name;
	Label *age;
	Label *mass;
	Label *pos;
	Label *tile;
	Label *goal;
	Panel *needs;
	Panel panel;

	Meter *health_meter;
	std::vector<Meter *> need_meters;

};

}
}

#endif
