#ifndef BLOBS_WORLD_BODY_HPP_
#define BLOBS_WORLD_BODY_HPP_

#include <vector>


namespace blobs {
namespace app {
	class Assets;
}
namespace graphics {
	class Viewport;
}
namespace world {

class Body {

public:
	Body();
	~Body();

	Body(const Body &) = delete;
	Body &operator =(const Body &) = delete;

	Body(Body &&) = delete;
	Body &operator =(Body &&) = delete;

public:
	bool HasParent() const { return parent; }
	Body &Parent() { return *parent; }
	const Body &Parent() const { return *parent; }
	void SetParent(Body &);
	void UnsetParent();

	virtual void Draw(app::Assets &, graphics::Viewport &) { }

private:
	void AddChild(Body &);
	void RemoveChild(Body &);

private:
	Body *parent;
	std::vector<Body *> children;
	double mass;
	double radius;

};

}
}

#endif
