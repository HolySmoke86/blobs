#include "MasterState.hpp"

#include "Application.hpp"
#include "../creature/Creature.hpp"
#include "../graphics/Viewport.hpp"
#include "../math/const.hpp"
#include "../world/Body.hpp"
#include "../world/Planet.hpp"
#include "../world/Simulation.hpp"
#include "../world/Sun.hpp"

#include <glm/gtx/vector_angle.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/transform.hpp>

#include <iostream>
#include <glm/gtx/io.hpp>


namespace blobs {
namespace app {

MasterState::MasterState(Assets &assets, world::Simulation &sim) noexcept
: State()
, assets(assets)
, sim(sim)
, cam(**sim.Suns().begin())
, cam_pos(0.0, 0.0, 1.0)
, cam_tgt_pos(0.0, 0.0, 1.0)
, cam_focus(0.0)
, cam_tgt_focus(0.0)
, cam_up(0.0, 1.0, 0.0)
, cam_tgt_up(0.0, 1.0, 0.0)
, cam_dist(5.0)
, cam_orient(PI * 0.375, PI * 0.25, 0.0)
, cam_dragging(false)
, shown_creature(nullptr)
, shown_body(nullptr)
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


void MasterState::Show(creature::Creature &c) noexcept {
	cam.Reference(c.GetSituation().GetPlanet());
	cam.TrackOrientation(true);
	cam_orient.x = std::max(0.0, cam_orient.x);
	cp.Show(c);
	bp.Hide();
	tp.SetBody(c.GetSituation().GetPlanet());
	shown_creature = &c;
	shown_body = nullptr;
}

void MasterState::Show(world::Body &b) noexcept {
	cam.Reference(b);
	cam.TrackOrientation(false);
	bp.Show(b);
	cp.Hide();
	tp.SetBody(b);
	shown_creature = nullptr;
	shown_body = &b;
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
	assets.shaders.sky_box.Activate();
	assets.shaders.sky_box.SetVP(cam.View() * cam.Universe(), cam.Projection());
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
	if (max_tick == 0) {
		// drop remaining
		remain = 0;
	}
}

void MasterState::Tick() {
	constexpr double dt = 0.01666666666666666666666666666666;
	if (!paused) {
		sim.Tick(dt);
	}
	remain -= FrameMS();
	thirds = (thirds + 1) % 3;

	// determine where camera should be
	double actual_dist = cam_dist;
	if (shown_creature) {
		cam_tgt_focus = shown_creature->GetSituation().Position();
		cam_tgt_up = glm::normalize(cam_tgt_focus);
		actual_dist += shown_creature->Size();
	} else if (shown_body) {
		cam_tgt_focus = glm::dvec3(0.0);
		cam_tgt_up = glm::dvec3(0.0, 1.0, 0.0);
		actual_dist += shown_body->Radius();
	}

	glm::dvec3 dir(0.0, 0.0, -actual_dist);
	glm::dvec3 ref_dir(glm::normalize(glm::cross(cam_tgt_up, glm::dvec3(-cam_tgt_up.z, cam_tgt_up.x, cam_tgt_up.y))));
	dir =
		glm::dmat3(ref_dir, cam_tgt_up, glm::cross(ref_dir, cam_tgt_up))
		* glm::dmat3(glm::eulerAngleYX(-cam_orient.y, -cam_orient.x))
		* dir;
	cam_tgt_up = glm::rotate(cam_tgt_up, cam_orient.z, glm::normalize(-dir));
	cam_tgt_pos = cam_tgt_focus - dir;

	// approach target location
	glm::dvec3 cam_pos_diff(cam_tgt_pos - cam_pos);
	if (glm::length2(cam_pos_diff) > 0.000001) {
		cam_pos += cam_pos_diff * 0.25;
	} else {
		cam_pos = cam_tgt_pos;
	}

	glm::dvec3 cam_focus_diff(cam_tgt_focus - cam_focus);
	if (glm::length2(cam_focus_diff) > 0.000001) {
		cam_focus += cam_focus_diff * 0.25;
	} else {
		cam_focus = cam_tgt_focus;
	}

	double cam_up_diff = glm::angle(cam_up, cam_tgt_up);
	if (cam_up_diff > 0.001) {
		cam_up = glm::rotate(cam_up, cam_up_diff * 0.25, glm::normalize(glm::cross(cam_up, cam_tgt_up)));
	} else {
		cam_up = cam_tgt_up;
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

		shown_creature = nullptr;
		double closest_dist = std::numeric_limits<double>::infinity();
		for (creature::Creature *c : sim.LiveCreatures()) {
			glm::dvec3 normal(0.0);
			double dist = 0.0;
			if (Intersect(ray, c->CollisionBounds(), glm::dmat4(cam.Model(c->GetSituation().GetPlanet())) * c->CollisionTransform(), normal, dist)
				&& dist < closest_dist) {
				shown_creature = c;
				closest_dist = dist;
			}
		}

		shown_body = nullptr;
		for (world::Body *b : sim.Bodies()) {
			glm::dvec3 normal(0.0);
			double dist = 0.0;
			if (Intersect(ray, glm::dmat4(cam.Model(*b)) * b->CollisionBounds(), normal, dist) && dist < closest_dist) {
				shown_creature = nullptr;
				shown_body = b;
				closest_dist = dist;
			}
		}

		if (shown_creature) {
			Show(*shown_creature);
		} else if (shown_body) {
			Show(*shown_body);
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
		cam_orient.x = glm::clamp(cam_orient.x + double(e.yrel) * pitch_scale, shown_creature ? 0.0 : PI * -0.499, PI * 0.499);
		cam_orient.y = std::fmod(cam_orient.y + double(e.xrel) * yaw_scale, PI * 2.0);
	}
}

void MasterState::OnMouseWheel(const SDL_MouseWheelEvent &e) {
	constexpr double roll_scale = PI * 0.0625;
	constexpr double zoom_scale = -1.0;
	constexpr double zoom_base = 1.125;
	cam_orient.z = glm::clamp(cam_orient.z + double(e.x) * roll_scale, PI * -0.5, PI * 0.5);
	cam_dist = std::max(0.125, cam_dist * std::pow(zoom_base, double(e.y) * zoom_scale));
}

void MasterState::OnRender(graphics::Viewport &viewport) {
	cam.LookAt(glm::vec3(cam_pos), glm::vec3(cam_focus), glm::vec3(cam_up));
	assets.shaders.planet_surface.Activate();
	assets.shaders.planet_surface.SetV(cam.View());
	assets.shaders.sky_box.Activate();
	assets.shaders.sky_box.SetV(cam.View() * cam.Universe());
	assets.shaders.sun_surface.Activate();
	assets.shaders.sun_surface.SetV(cam.View());
	assets.shaders.creature_skin.Activate();
	assets.shaders.creature_skin.SetV(cam.View());

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

	assets.shaders.sky_box.Activate();
	assets.shaders.sky_box.SetTexture(assets.textures.sky);
	assets.shaders.sky_box.Draw();

	viewport.ClearDepth();
	bp.Draw(viewport);
	cp.Draw(viewport);
	rp.Draw(viewport);
	tp.Draw(viewport);
}

}
}
