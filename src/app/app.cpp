#include "Application.hpp"
#include "State.hpp"

#include <SDL.h>


namespace blobs {
namespace app {

Application::Application()
: states() {
}

Application::~Application() {
}


void Application::PushState(State *s) {
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
	GetState().Render();
}

void Application::HandleEvents() {
	SDL_Event event;
	while (HasState() && SDL_PollEvent(&event)) {
		GetState().Handle(event);
	}
}

void State::Handle(const SDL_Event &event) {
	switch (event.type) {
		case SDL_WINDOWEVENT:
			Handle(event.window);
			break;
		default:
			OnEvent(event);
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
			//env.viewport.Resize(event.data1, event.data2);
			OnResize();
			break;
		default:
			break;
	}
}

void State::Update(int dt) {
	OnUpdate(dt);
}

void State::Render() {
	OnRender();
}

}
}
