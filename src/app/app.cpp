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
, tile_path(path + "tiles/") {
	graphics::Format format;
	textures.tiles.Bind();
	textures.tiles.Reserve(256, 256, 9, format);
	LoadTileTexture("1", textures.tiles, 0);
	LoadTileTexture("2", textures.tiles, 1);
	LoadTileTexture("3", textures.tiles, 2);
	LoadTileTexture("4", textures.tiles, 3);
	LoadTileTexture("5", textures.tiles, 4);
	LoadTileTexture("6", textures.tiles, 5);
	LoadTileTexture("7", textures.tiles, 6);
	LoadTileTexture("8", textures.tiles, 7);
	LoadTileTexture("9", textures.tiles, 8);
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

}
}
