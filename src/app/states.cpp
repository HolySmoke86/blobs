#include "MasterState.hpp"

#include "../world/Body.hpp"
#include "../world/Simulation.hpp"

#include <glm/gtx/transform.hpp>


namespace blobs {
namespace app {

MasterState::MasterState(Assets &assets, world::Simulation &sim) noexcept
: State()
, assets(assets)
, sim(sim)
, reference(&sim.Root())
, cam()
, remain(0)
, thirds(0)
, paused(false) {
	// sunset view: standing in the center of surface 0 (+Z), looking west (-X)
	//cam.View(glm::lookAt(glm::vec3(0.0f, 0.0f, 5.6f), glm::vec3(-1.0f, 0.0f, 5.6f), glm::vec3(0.0f, 0.0f, 1.0f)));
	// sunrise view: standing in the center of surface 0 (+Z), looking east (+X)
	cam.View(glm::lookAt(glm::vec3(0.0f, 0.0f, 5.6f), glm::vec3(1.0f, 0.0f, 5.6f), glm::vec3(0.0f, 0.0f, 1.0f)));
	// far out, looking at planet
	//cam.View(glm::lookAt(glm::vec3(10.0f, 10.0f, 50.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f)));
}

MasterState::~MasterState() noexcept {
}


void MasterState::OnResize(int w, int h) {
	cam.Aspect(float(w), float(h));
}

void MasterState::OnUpdate(int dt) {
	remain += dt;
	while (remain >= FrameMS()) {
		Tick();
	}
}

void MasterState::Tick() {
	if (!paused) {
		sim.Tick();
	}
	remain -= FrameMS();
	thirds = (thirds + 1) % 3;
}

int MasterState::FrameMS() const noexcept {
	return thirds == 0 ? 16 : 17;
}


void MasterState::OnKeyDown(const SDL_KeyboardEvent &e) {
	if (e.keysym.sym == SDLK_p) {
		paused = !paused;
	}
}

void MasterState::OnRender(graphics::Viewport &viewport) {
	glm::dmat4 ppos = reference->InverseTransform() * reference->ToParent();
	assets.shaders.planet_surface.Activate();
	assets.shaders.planet_surface.SetTexture(assets.textures.tiles);

	assets.shaders.planet_surface.SetMVP(glm::mat4(1.0f), cam.View(), cam.Projection());
	assets.shaders.planet_surface.SetLight(glm::vec3(cam.View() * ppos[3]), glm::vec3(1.0f, 1.0f, 1.0f), 2.0e4f);
	reference->Draw(assets, viewport);

	world::Body *child = reference->Children()[0];
	assets.shaders.planet_surface.SetMVP(reference->InverseTransform() * child->FromParent() * child->LocalTransform(), cam.View(), cam.Projection());
	child->Draw(assets, viewport);

	assets.shaders.sun_surface.Activate();
	assets.shaders.sun_surface.SetMVP(ppos * reference->Parent().LocalTransform(), cam.View(), cam.Projection());
	assets.shaders.sun_surface.SetLight(glm::vec3(1.0f, 1.0f, 1.0f), 2.0e4f);
	assets.shaders.sun_surface.Draw();
}

}
}
