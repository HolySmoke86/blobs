#ifndef BLOBS_UI_BODYPANEL_HPP_
#define BLOBS_UI_BODYPANEL_HPP_

#include "Panel.hpp"

namespace blobs {
namespace app {
	struct Assets;
}
namespace graphics {
	class Viewport;
}
namespace world {
	class Body;
}
namespace ui {

class Label;

class BodyPanel {

public:
	explicit BodyPanel(app::Assets &);
	~BodyPanel();

	BodyPanel(const BodyPanel &) = delete;
	BodyPanel &operator =(const BodyPanel &) = delete;

	BodyPanel(BodyPanel &&) = delete;
	BodyPanel &operator =(BodyPanel &&) = delete;

public:
	void Show(world::Body &);
	void Hide() noexcept;

	bool Shown() const noexcept { return body; }
	const world::Body &GetBody() const noexcept { return *body; }

	void ZIndex(float z) noexcept { panel.ZIndex(z); }

	void Draw(graphics::Viewport &) noexcept;

private:
	app::Assets &assets;
	world::Body *body;

	Label *name;
	Label *mass;
	Label *radius;
	Label *soi;
	Label *operiod;
	Label *rperiod;
	Label *atm;
	Label *sma;
	Label *ecc;
	Label *inc;
	Label *asc;
	Label *arg;
	Label *mna;
	Panel panel;

};

}
}

#endif
