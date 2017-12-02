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

	bool Shown() const noexcept { return c; }
	const creature::Creature &GetCreature() const noexcept { return *c; }

	void Draw(app::Assets &, graphics::Viewport &) noexcept;

private:
	void CreateNeeds();

private:
	const app::Assets &assets;
	creature::Creature *c;

	Label *name;
	Label *born;
	Label *age;
	Label *mass;
	Label *pos;
	Label *vel;
	Label *dir;
	Label *tile;
	Label *goal;
	Meter *stats[7];
	Label *props[8];
	Panel panel;

};

}
}

#endif
