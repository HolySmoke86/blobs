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
, thirds(0) {
	cam.View(glm::lookAt(glm::vec3(2.0f, 3.0f, 10.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f)));
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
	sim.Tick();
	remain -= FrameMS();
	thirds = (thirds + 1) % 3;
}

int MasterState::FrameMS() const noexcept {
	return thirds == 0 ? 16 : 17;
}


void MasterState::OnRender(graphics::Viewport &viewport) {
	glm::mat4 ppos = reference->ToParent();
	assets.shaders.planet_surface.Activate();
	assets.shaders.planet_surface.SetMVP(glm::mat4(1.0f), cam.View(), cam.Projection());
	assets.shaders.planet_surface.SetTexture(assets.textures.tiles);
	assets.shaders.planet_surface.SetLight(glm::vec3(cam.View() * ppos[3]), glm::vec3(1.0f, 1.0f, 1.0f), 100.0f);
	reference->Draw(assets, viewport);
}

}
}
