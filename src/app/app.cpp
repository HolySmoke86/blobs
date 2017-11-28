#include "Application.hpp"
#include "Assets.hpp"
#include "State.hpp"

#include "init.hpp"
#include "../graphics/Viewport.hpp"
#include "../io/Token.hpp"
#include "../io/TokenStreamReader.hpp"

#include <fstream>
#include <SDL.h>
#include <SDL_image.h>

using std::string;


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
, data_path(path + "data/")
, font_path(path + "fonts/")
, skin_path(path + "skins/")
, tile_path(path + "tiles/")
, random(0x6283B64CEFE47925)
, fonts{
	graphics::Font(font_path + "DejaVuSans.ttf", 32),
	graphics::Font(font_path + "DejaVuSans.ttf", 24),
	graphics::Font(font_path + "DejaVuSans.ttf", 16)
} {
	{
		std::ifstream resource_file(data_path + "resources");
		io::TokenStreamReader resource_reader(resource_file);
		ReadResources(resource_reader);
	}

	{
		std::ifstream tile_file(data_path + "tile_types");
		io::TokenStreamReader tile_reader(tile_file);
		ReadTileTypes(tile_reader);
	}


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
	textures.tiles.FilterTrilinear();

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
	textures.skins.FilterTrilinear();
}

Assets::~Assets() {
}

void Assets::ReadResources(io::TokenStreamReader &in) {
	while (in.HasMore()) {
		string name;
		in.ReadIdentifier(name);
		in.Skip(io::Token::EQUALS);

		int id = 0;
		if (data.resources.Has(name)) {
			id = data.resources[name].id;
		} else {
			world::Resource res;
			res.name = name;
			id = data.resources.Add(res);
		}

		in.Skip(io::Token::ANGLE_BRACKET_OPEN);
		while (in.Peek().type != io::Token::ANGLE_BRACKET_CLOSE) {
			in.ReadIdentifier(name);
			in.Skip(io::Token::EQUALS);
			if (name == "label") {
				in.ReadString(data.resources[id].label);
			} else if (name == "density") {
				data.resources[id].density = in.GetDouble();
			} else if (name == "state") {
				in.ReadIdentifier(name);
				if (name == "solid") {
					data.resources[id].state = world::Resource::SOLID;
				} else if (name == "liquid") {
					data.resources[id].state = world::Resource::LIQUID;
				} else if (name == "gas") {
					data.resources[id].state = world::Resource::GAS;
				} else if (name == "plasma") {
					data.resources[id].state = world::Resource::PLASMA;
				} else {
					throw std::runtime_error("unknown resource state '" + name + "'");
				}
			} else if (name == "base_color") {
				in.ReadVec(data.resources[id].base_color);
			} else {
				throw std::runtime_error("unknown resource property '" + name + "'");
			}
			in.Skip(io::Token::SEMICOLON);
		}
		in.Skip(io::Token::ANGLE_BRACKET_CLOSE);
		in.Skip(io::Token::SEMICOLON);
	}
}

void Assets::ReadTileTypes(io::TokenStreamReader &in) {
	while (in.HasMore()) {
		string name;
		in.ReadIdentifier(name);
		in.Skip(io::Token::EQUALS);

		int id = 0;
		if (data.tile_types.Has(name)) {
			id = data.tile_types[name].id;
		} else {
			world::TileType type;
			type.name = name;
			id = data.tile_types.Add(type);
		}

		in.Skip(io::Token::ANGLE_BRACKET_OPEN);
		while (in.Peek().type != io::Token::ANGLE_BRACKET_CLOSE) {
			in.ReadIdentifier(name);
			in.Skip(io::Token::EQUALS);
			if (name == "label") {
				in.ReadString(data.tile_types[id].label);
			} else if (name == "texture") {
				data.tile_types[id].texture = in.GetInt();
			} else if (name == "yield") {
				in.Skip(io::Token::BRACKET_OPEN);
				while (in.Peek().type != io::Token::BRACKET_CLOSE) {
					world::TileType::Yield yield;
					in.Skip(io::Token::ANGLE_BRACKET_OPEN);
					while (in.Peek().type != io::Token::ANGLE_BRACKET_CLOSE) {
						in.ReadIdentifier(name);
						in.Skip(io::Token::EQUALS);
						if (name == "resource") {
							in.ReadIdentifier(name);
							yield.resource = data.resources[name].id;
						} else if (name == "ubiquity") {
							yield.ubiquity = in.GetDouble();
						} else {
							throw std::runtime_error("unknown tile type yield property '" + name + "'");
						}
						in.Skip(io::Token::SEMICOLON);
					}
					in.Skip(io::Token::ANGLE_BRACKET_CLOSE);
					data.tile_types[id].resources.push_back(yield);
					if (in.Peek().type == io::Token::COMMA) {
						in.Skip(io::Token::COMMA);
					}
				}
				in.Skip(io::Token::BRACKET_CLOSE);
			} else {
				throw std::runtime_error("unknown tile type property '" + name + "'");
			}
			in.Skip(io::Token::SEMICOLON);
		}
		in.Skip(io::Token::ANGLE_BRACKET_CLOSE);
		in.Skip(io::Token::SEMICOLON);
	}
}

void Assets::LoadTileTexture(const string &name, graphics::ArrayTexture &tex, int layer) const {
	string path = tile_path + name + ".png";
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

void Assets::LoadSkinTexture(const string &name, graphics::ArrayTexture &tex, int layer) const {
	string path = skin_path + name + ".png";
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
