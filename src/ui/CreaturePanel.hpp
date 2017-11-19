#ifndef BLOBS_UI_CREATUREPANEL_HPP_
#define BLOBS_UI_CREATUREPANEL_HPP_

#include "Label.hpp"


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
	creature::Creature *c;

	Label name;

};

}
}

#endif
