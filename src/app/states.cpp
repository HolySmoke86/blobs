#include "MasterState.hpp"

#include "Application.hpp"
#include "../creature/Creature.hpp"
#include "../graphics/Viewport.hpp"
#include "../math/const.hpp"
#include "../world/Body.hpp"
#include "../world/Planet.hpp"
#include "../world/Simulation.hpp"
#include "../world/Sun.hpp"

#include <glm/gtx/transform.hpp>

#include <iostream>
#include <glm/gtx/io.hpp>


namespace blobs {
namespace app {

MasterState::MasterState(Assets &assets, world::Simulation &sim) noexcept
: State()
, assets(assets)
, sim(sim)
, cam(sim.Root())
, cam_dist(5.0)
, cam_tgt_dist(5.0)
, cam_orient(PI * 0.375, PI * 0.25, 0.0)
, cam_dragging(false)
, bp(assets)
, cp(assets)
, rp(sim)
, tp(sim)
, remain(0)
, thirds(0)
, paused(false) {
	bp.ZIndex(10.0f);
	cp.ZIndex(20.0f);
	rp.ZIndex(30.0f);
	tp.ZIndex(40.0f);
}

MasterState::~MasterState() noexcept {
}


void MasterState::OnResize(int w, int h) {
	assets.shaders.canvas.Activate();
	assets.shaders.canvas.Resize(float(w), float(h));
	assets.shaders.alpha_sprite.Activate();
	assets.shaders.alpha_sprite.SetVP(glm::mat4(1.0f), glm::ortho(0.0f, float(w), float(h), 0.0f, 1.0e4f, -1.0e4f));

	cam.Aspect(float(w), float(h));
	assets.shaders.planet_surface.Activate();
	assets.shaders.planet_surface.SetVP(cam.View(), cam.Projection());
	assets.shaders.sun_surface.Activate();
	assets.shaders.sun_surface.SetVP(cam.View(), cam.Projection());
	assets.shaders.creature_skin.Activate();
	assets.shaders.creature_skin.SetVP(cam.View(), cam.Projection());
}

void MasterState::OnUpdate(int dt) {
	remain += dt;
#ifdef NDEBUG
	int max_tick = 10;
#else
	// one tick per frame when debugging so pausing execution doesn't result in more ticks
	int max_tick = 1;
#endif
	while (remain >= FrameMS() && max_tick > 0) {
		Tick();
		--max_tick;
	}
}

void MasterState::Tick() {
	constexpr double dt = 0.01666666666666666666666666666666;
	if (!paused) {
		sim.Tick(dt);
	}
	remain -= FrameMS();
	thirds = (thirds + 1) % 3;

	double cam_diff = cam_tgt_dist - cam_dist;
	if (std::abs(cam_diff) > 0.001) {
		cam_dist += cam_diff * 0.25;
	} else {
		cam_dist = cam_tgt_dist;
	}
}

int MasterState::FrameMS() const noexcept {
	return thirds == 0 ? 16 : 17;
}


void MasterState::OnKeyDown(const SDL_KeyboardEvent &e) {
	if (e.keysym.sym == SDLK_p) {
		paused = !paused;
	} else if (e.keysym.sym == SDLK_F1) {
		rp.Toggle();
	}
}

void MasterState::OnMouseDown(const SDL_MouseButtonEvent &e) {
	if (e.button == SDL_BUTTON_RIGHT) {
		SDL_SetRelativeMouseMode(SDL_TRUE);
		cam_dragging = true;
	}
}

void MasterState::OnMouseUp(const SDL_MouseButtonEvent &e) {
	if (e.button == SDL_BUTTON_LEFT) {
		glm::dmat4 inverse(glm::inverse(cam.Projection() * cam.View()));
		math::Ray ray(inverse * App().GetViewport().ShootPixel(e.x, e.y));

		creature::Creature *closest_creature = nullptr;
		double closest_dist = std::numeric_limits<double>::infinity();
		for (creature::Creature *c : sim.LiveCreatures()) {
			glm::dvec3 normal(0.0);
			double dist = 0.0;
			if (Intersect(ray, c->CollisionBounds(), glm::dmat4(cam.Model(c->GetSituation().GetPlanet())) * c->CollisionTransform(), normal, dist)
				&& dist < closest_dist) {
				closest_creature = c;
				closest_dist = dist;
			}
		}

		world::Body *closest_body = nullptr;
		for (world::Body *b : sim.Bodies()) {
			glm::dvec3 normal(0.0);
			double dist = 0.0;
			if (Intersect(ray, glm::dmat4(cam.Model(*b)) * b->CollisionBounds(), normal, dist) && dist < closest_dist) {
				closest_creature = nullptr;
				closest_body = b;
				closest_dist = dist;
			}
		}

		if (closest_creature) {
			cp.Show(*closest_creature);
			bp.Hide();
		} else if (closest_body) {
			bp.Show(*closest_body);
			cp.Hide();
		} else {
			cp.Hide();
			bp.Hide();
		}
	} else if (e.button == SDL_BUTTON_RIGHT) {
		SDL_SetRelativeMouseMode(SDL_FALSE);
		cam_dragging = false;
	}
}

void MasterState::OnMouseMotion(const SDL_MouseMotionEvent &e) {
	constexpr double pitch_scale = PI * 0.001;
	constexpr double yaw_scale = PI * 0.002;
	if (cam_dragging) {
		cam_orient.x = glm::clamp(cam_orient.x + double(e.yrel) * pitch_scale, 0.0, PI * 0.499);
		cam_orient.y = std::fmod(cam_orient.y + double(e.xrel) * yaw_scale, PI * 2.0);
	}
}

void MasterState::OnMouseWheel(const SDL_MouseWheelEvent &e) {
	constexpr double roll_scale = PI * 0.0625;
	constexpr double zoom_scale = -1.0;
	constexpr double zoom_base = 1.125;
	cam_orient.z = glm::clamp(cam_orient.z + double(e.x) * roll_scale, PI * -0.5, PI * 0.5);
	if (cp.Shown()) {
		cam_tgt_dist = std::max(cp.GetCreature().Size() * 2.0, cam_tgt_dist * std::pow(zoom_base, double(e.y) * zoom_scale));
	} else {
		cam_tgt_dist = std::max(1.0, cam_tgt_dist * std::pow(zoom_base, double(e.y) * zoom_scale));
	}
}

void MasterState::OnRender(graphics::Viewport &viewport) {
	if (cp.Shown()) {
		cam.Radial(cp.GetCreature(), cam_dist, cam_orient);
		assets.shaders.planet_surface.Activate();
		assets.shaders.planet_surface.SetV(cam.View());
		assets.shaders.sun_surface.Activate();
		assets.shaders.sun_surface.SetV(cam.View());
		assets.shaders.creature_skin.Activate();
		assets.shaders.creature_skin.SetV(cam.View());
	}

	int num_lights = 0;
	for (auto sun : sim.Suns()) {
		glm::vec3 pos(cam.View() * cam.Model(*sun)[3]);
		assets.shaders.planet_surface.Activate();
		assets.shaders.planet_surface.SetLight(num_lights, pos, glm::vec3(sun->Color()), float(sun->Luminosity()));
		assets.shaders.creature_skin.Activate();
		assets.shaders.creature_skin.SetLight(num_lights, pos, glm::vec3(sun->Color()), float(sun->Luminosity()));
		++num_lights;
		if (num_lights >= graphics::PlanetSurface::MAX_LIGHTS || num_lights >= graphics::CreatureSkin::MAX_LIGHTS) {
			break;
		}
	}
	for (auto planet : sim.Planets()) {
		// TODO: indirect light from planets, calculate strength and get color somehow
		glm::vec3 pos(cam.View() * cam.Model(*planet)[3]);
		glm::vec3 col(1.0f, 1.0f, 1.0f);
		float str = 10.0f;
		assets.shaders.planet_surface.Activate();
		assets.shaders.planet_surface.SetLight(num_lights, pos, col, str);
		assets.shaders.creature_skin.Activate();
		assets.shaders.creature_skin.SetLight(num_lights, pos, col, str);
		++num_lights;
		if (num_lights >= graphics::PlanetSurface::MAX_LIGHTS || num_lights >= graphics::CreatureSkin::MAX_LIGHTS) {
			break;
		}
	}
	assets.shaders.planet_surface.Activate();
	assets.shaders.planet_surface.SetNumLights(num_lights);
	assets.shaders.creature_skin.Activate();
	assets.shaders.creature_skin.SetNumLights(num_lights);

	assets.shaders.planet_surface.Activate();
	assets.shaders.planet_surface.SetTexture(assets.textures.tiles);
	for (auto planet : sim.Planets()) {
		assets.shaders.planet_surface.SetM(cam.Model(*planet));
		planet->Draw(assets, viewport);
	}

	assets.shaders.sun_surface.Activate();
	for (auto sun : sim.Suns()) {
		double sun_radius = sun->Radius();
		assets.shaders.sun_surface.SetM(
			cam.Model(*sun) * glm::scale(glm::vec3(sun_radius, sun_radius, sun_radius)));
		assets.shaders.sun_surface.SetLight(glm::vec3(sun->Color()), float(sun->Luminosity()));
		assets.shaders.sun_surface.Draw();
	}

	assets.shaders.creature_skin.Activate();
	assets.shaders.creature_skin.SetTexture(assets.textures.skins);
	// TODO: extend to nearby bodies as well
	for (auto c : cam.Reference().Creatures()) {
		assets.shaders.creature_skin.SetM(cam.Model(cam.Reference()) * glm::mat4(c->LocalTransform()));
		assets.shaders.creature_skin.SetBaseColor(glm::vec3(c->BaseColor()));
		assets.shaders.creature_skin.SetHighlightColor(glm::vec4(c->HighlightColor()));
		c->Draw(viewport);
	}

	viewport.ClearDepth();
	bp.Draw(viewport);
	cp.Draw(viewport);
	rp.Draw(viewport);
	tp.Draw(viewport);
}

}
}
