#include "MasterState.hpp"

#include "../world/Body.hpp"
#include "../world/Planet.hpp"
#include "../world/Simulation.hpp"
#include "../world/Sun.hpp"

#include <glm/gtx/transform.hpp>


namespace blobs {
namespace app {

MasterState::MasterState(Assets &assets, world::Simulation &sim) noexcept
: State()
, assets(assets)
, sim(sim)
, cam(sim.Root())
, remain(0)
, thirds(0)
, paused(false) {
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
	glm::dmat4 ppos = cam.Model(**sim.Suns().begin());
	assets.shaders.planet_surface.Activate();
	assets.shaders.planet_surface.SetTexture(assets.textures.tiles);
	assets.shaders.planet_surface.SetLight(glm::vec3(cam.View() * ppos[3]), glm::vec3(1.0f, 1.0f, 1.0f), 1.0e6f);

	for (auto planet : sim.Planets()) {
		assets.shaders.planet_surface.SetMVP(cam.Model(*planet), cam.View(), cam.Projection());
		planet->Draw(assets, viewport);
	}

	assets.shaders.sun_surface.Activate();
	for (auto sun : sim.Suns()) {
		double sun_radius = sun->Radius();
		assets.shaders.sun_surface.SetMVP(
			cam.Model(*sun) * glm::scale(glm::vec3(sun_radius, sun_radius, sun_radius)),
			cam.View(), cam.Projection());
		assets.shaders.sun_surface.SetLight(glm::vec3(1.0f, 1.0f, 1.0f), 1.0e6f);
		assets.shaders.sun_surface.Draw();
	}
}

}
}
