#include "Application.hpp"
#include "Assets.hpp"
#include "State.hpp"

#include "init.hpp"
#include "../graphics/Viewport.hpp"

#include <SDL.h>
#include <SDL_image.h>


namespace blobs {
namespace app {

Application::Application(Window &win, graphics::Viewport &vp)
: window(win)
, viewport(vp)
, states() {
}

Application::~Application() {
}


void Application::PushState(State *s) {
	s->app = this;
	if (!states.empty()) {
		states.top()->OnPause();
	}
	states.emplace(s);
	++s->ref_count;
	if (s->ref_count == 1) {
		s->OnEnter();
	}
	s->OnResume();
}

State *Application::PopState() {
	State *s = states.top();
	states.pop();
	s->OnPause();
	s->OnExit();
	if (!states.empty()) {
		states.top()->OnResume();
	}
	return s;
}

State *Application::SwitchState(State *s_new) {
	s_new->app = this;
	State *s_old = states.top();
	states.top() = s_new;
	--s_old->ref_count;
	++s_new->ref_count;
	s_old->OnPause();
	if (s_old->ref_count == 0) {
		s_old->OnExit();
	}
	if (s_new->ref_count == 1) {
		s_new->OnEnter();
	}
	s_new->OnResume();
	return s_old;
}

State &Application::GetState() {
	return *states.top();
}

bool Application::HasState() const noexcept {
	return !states.empty();
}


void Application::Run() {
	Uint32 last = SDL_GetTicks();
	while (HasState()) {
		Uint32 now = SDL_GetTicks();
		int delta = now - last;
		Loop(delta);
		last = now;
	}
}

void Application::Loop(int dt) {
	HandleEvents();
	if (!HasState()) return;
	GetState().Update(dt);
	if (!HasState()) return;
	viewport.Clear();
	GetState().Render(viewport);
	window.Flip();
}

void Application::HandleEvents() {
	SDL_Event event;
	while (HasState() && SDL_PollEvent(&event)) {
		if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_RESIZED) {
			viewport.Resize(event.window.data1, event.window.data2);
		}
		GetState().Handle(event);
	}
}

void State::Handle(const SDL_Event &event) {
	switch (event.type) {
		case SDL_KEYDOWN:
			OnKeyDown(event.key);
			break;
		case SDL_KEYUP:
			OnKeyUp(event.key);
			break;
		case SDL_MOUSEBUTTONDOWN:
			OnMouseDown(event.button);
			break;
		case SDL_MOUSEBUTTONUP:
			OnMouseUp(event.button);
			break;
		case SDL_MOUSEMOTION:
			OnMouseMotion(event.motion);
			break;
		case SDL_MOUSEWHEEL:
			OnMouseWheel(event.wheel);
			break;
		case SDL_QUIT:
			OnQuit();
			break;
		case SDL_WINDOWEVENT:
			Handle(event.window);
			break;
		default:
			// ignore
			break;
	}
}

void State::Handle(const SDL_WindowEvent &event) {
	switch (event.event) {
		case SDL_WINDOWEVENT_FOCUS_GAINED:
			OnFocus();
			break;
		case SDL_WINDOWEVENT_FOCUS_LOST:
			OnBlur();
			break;
		case SDL_WINDOWEVENT_RESIZED:
			OnResize(event.data1, event.data2);
			break;
		default:
			break;
	}
}

void State::Update(int dt) {
	OnUpdate(dt);
}

void State::Render(graphics::Viewport &viewport) {
	OnRender(viewport);
}

void State::OnQuit() {
	while (App().HasState()) {
		App().PopState();
	}
}


Assets::Assets()
: path("assets/")
, tile_path(path + "tiles/")
, skin_path(path + "skins/") {
	data.resources.Add({ "air", "Air", 0 });
	data.resources.Add({ "biomass", "Biomass", 0 });
	data.resources.Add({ "dirt", "Dirt", 0 });
	data.resources.Add({ "ice", "Ice", 0 });
	data.resources.Add({ "rock", "Rock", 0 });
	data.resources.Add({ "sand", "Sand", 0 });
	data.resources.Add({ "water", "Water", 0 });
	data.resources.Add({ "wood", "Wood", 0 });

	data.tiles.Add({ "algae",    "Algae",    0,  0 });
	data.tiles.Add({ "desert",   "Desert",   0,  1 });
	data.tiles.Add({ "forest",   "Forest",   0,  2 });
	data.tiles.Add({ "grass",    "Grass",    0,  3 });
	data.tiles.Add({ "ice",      "Ice",      0,  4 });
	data.tiles.Add({ "jungle",   "Jungle",   0,  5 });
	data.tiles.Add({ "mountain", "Mountain", 0,  6 });
	data.tiles.Add({ "ocean",    "Ocean",    0,  7 });
	data.tiles.Add({ "rock",     "Rock",     0,  8 });
	data.tiles.Add({ "sand",     "Sand",     0,  9 });
	data.tiles.Add({ "taiga",    "Taiga",    0, 10 });
	data.tiles.Add({ "tundra",   "Tundra",   0, 11 });
	data.tiles.Add({ "water",    "Water",    0, 12 });
	data.tiles.Add({ "wheat",    "Wheat",    0, 13 });

	data.tiles["algae"]   .resources.push_back({ data.resources["water"].id,   1.0  });
	data.tiles["algae"]   .resources.push_back({ data.resources["biomass"].id, 0.5  });
	data.tiles["desert"]  .resources.push_back({ data.resources["sand"].id,    1.0  });
	data.tiles["forest"]  .resources.push_back({ data.resources["wood"].id,    1.0  });
	data.tiles["forest"]  .resources.push_back({ data.resources["dirt"].id,    0.5  });
	data.tiles["grass"]   .resources.push_back({ data.resources["dirt"].id,    0.5  });
	data.tiles["grass"]   .resources.push_back({ data.resources["biomass"].id, 0.25 });
	data.tiles["grass"]   .resources.push_back({ data.resources["water"].id,   0.25 });
	data.tiles["ice"]     .resources.push_back({ data.resources["ice"].id,     1.0  });
	data.tiles["ice"]     .resources.push_back({ data.resources["water"].id,   0.25 });
	data.tiles["jungle"]  .resources.push_back({ data.resources["wood"].id,    0.5  });
	data.tiles["jungle"]  .resources.push_back({ data.resources["biomass"].id, 0.5  });
	data.tiles["mountain"].resources.push_back({ data.resources["rock"].id,    1.0  });
	data.tiles["ocean"]   .resources.push_back({ data.resources["water"].id,   1.0  });
	data.tiles["rock"]    .resources.push_back({ data.resources["rock"].id,    1.0  });
	data.tiles["sand"]    .resources.push_back({ data.resources["sand"].id,    1.0  });
	data.tiles["taiga"]   .resources.push_back({ data.resources["wood"].id,    1.0  });
	data.tiles["taiga"]   .resources.push_back({ data.resources["water"].id,   0.5  });
	data.tiles["tundra"]  .resources.push_back({ data.resources["rock"].id,    1.0  });
	data.tiles["tundra"]  .resources.push_back({ data.resources["ice"].id,     0.5  });
	data.tiles["water"]   .resources.push_back({ data.resources["water"].id,   1.0  });
	data.tiles["water"]   .resources.push_back({ data.resources["biomass"].id, 0.25 });
	data.tiles["wheat"]   .resources.push_back({ data.resources["biomass"].id, 1.0  });
	data.tiles["wheat"]   .resources.push_back({ data.resources["water"].id,   0.25 });

	graphics::Format format;
	textures.tiles.Bind();
	textures.tiles.Reserve(256, 256, 14, format);
	LoadTileTexture("algae",    textures.tiles,  0);
	LoadTileTexture("desert",   textures.tiles,  1);
	LoadTileTexture("forest",   textures.tiles,  2);
	LoadTileTexture("grass",    textures.tiles,  3);
	LoadTileTexture("ice",      textures.tiles,  4);
	LoadTileTexture("jungle",   textures.tiles,  5);
	LoadTileTexture("mountain", textures.tiles,  6);
	LoadTileTexture("ocean",    textures.tiles,  7);
	LoadTileTexture("rock",     textures.tiles,  8);
	LoadTileTexture("sand",     textures.tiles,  9);
	LoadTileTexture("taiga",    textures.tiles, 10);
	LoadTileTexture("tundra",   textures.tiles, 11);
	LoadTileTexture("water",    textures.tiles, 12);
	LoadTileTexture("wheat",    textures.tiles, 13);

	textures.skins.Bind();
	textures.skins.Reserve(256, 256, 9, format);
	LoadSkinTexture("1", textures.skins, 0);
	LoadSkinTexture("2", textures.skins, 1);
	LoadSkinTexture("3", textures.skins, 2);
	LoadSkinTexture("4", textures.skins, 3);
	LoadSkinTexture("5", textures.skins, 4);
	LoadSkinTexture("6", textures.skins, 5);
	LoadSkinTexture("7", textures.skins, 6);
	LoadSkinTexture("8", textures.skins, 7);
	LoadSkinTexture("9", textures.skins, 8);
}

Assets::~Assets() {
}

void Assets::LoadTileTexture(const std::string &name, graphics::ArrayTexture &tex, int layer) const {
	std::string path = tile_path + name + ".png";
	SDL_Surface *srf = IMG_Load(path.c_str());
	if (!srf) {
		throw SDLError("IMG_Load");
	}
	try {
		tex.Data(layer, *srf);
	} catch (...) {
		SDL_FreeSurface(srf);
		throw;
	}
	SDL_FreeSurface(srf);
}

void Assets::LoadSkinTexture(const std::string &name, graphics::ArrayTexture &tex, int layer) const {
	std::string path = skin_path + name + ".png";
	SDL_Surface *srf = IMG_Load(path.c_str());
	if (!srf) {
		throw SDLError("IMG_Load");
	}
	try {
		tex.Data(layer, *srf);
	} catch (...) {
		SDL_FreeSurface(srf);
		throw;
	}
	SDL_FreeSurface(srf);
}

}
}
