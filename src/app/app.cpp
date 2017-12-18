#include "Application.hpp"
#include "Assets.hpp"
#include "State.hpp"

#include "init.hpp"
#include "../graphics/Viewport.hpp"
#include "../io/Token.hpp"
#include "../io/TokenStreamReader.hpp"
#include "../world/Planet.hpp"
#include "../world/Simulation.hpp"
#include "../world/Sun.hpp"

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
		s->OnResize(viewport.Width(), viewport.Height());
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
		s_new->OnResize(viewport.Width(), viewport.Height());
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
, sky_path(path + "skies/")
, tile_path(path + "tiles/")
, random(0x6283B64CEFE57925)
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
	LoadSkinTexture("plain", textures.skins, 0);
	LoadSkinTexture("stripes", textures.skins, 1);
	LoadSkinTexture("dots", textures.skins, 2);
	LoadSkinTexture("lines", textures.skins, 3);
	LoadSkinTexture("spots", textures.skins, 4);
	LoadSkinTexture("circles", textures.skins, 5);
	textures.skins.FilterTrilinear();

	textures.sky.Bind();
	LoadSkyTexture("blue", textures.sky);
	textures.sky.FilterTrilinear();
	textures.sky.WrapEdge();
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
			} else if (name == "energy") {
				data.resources[id].energy = in.GetDouble();
				data.resources[id].inverse_energy = 1.0 / data.resources[id].energy;
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
			} else if (name == "compatibility") {
				in.Skip(io::Token::ANGLE_BRACKET_OPEN);
				while (in.Peek().type != io::Token::ANGLE_BRACKET_CLOSE) {
					in.ReadIdentifier(name);
					int sub_id = 0;
					if (data.resources.Has(name)) {
						sub_id = data.resources[name].id;
					} else {
						world::Resource res;
						res.name = name;
						sub_id = data.resources.Add(res);
					}
					in.Skip(io::Token::COLON);
					double value = in.GetDouble();
					in.Skip(io::Token::SEMICOLON);
					data.resources[id].compatibility[sub_id] = value;
				}
				in.Skip(io::Token::ANGLE_BRACKET_CLOSE);
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

void Assets::LoadSkyTexture(const string &name, graphics::CubeMap &cm) const {
	string full = sky_path + name;
	string right = full + "-right.png";
	string left = full + "-left.png";
	string top = full + "-top.png";
	string bottom = full + "-bottom.png";
	string back = full + "-back.png";
	string front = full + "-front.png";

	SDL_Surface *srf = nullptr;

	if (!(srf = IMG_Load(right.c_str()))) throw SDLError("IMG_Load");
	try {
		cm.Data(graphics::CubeMap::RIGHT, *srf);
	} catch (...) {
		SDL_FreeSurface(srf);
		throw;
	}
	SDL_FreeSurface(srf);

	if (!(srf = IMG_Load(left.c_str()))) throw SDLError("IMG_Load");
	try {
		cm.Data(graphics::CubeMap::LEFT, *srf);
	} catch (...) {
		SDL_FreeSurface(srf);
		throw;
	}
	SDL_FreeSurface(srf);

	if (!(srf = IMG_Load(top.c_str()))) throw SDLError("IMG_Load");
	try {
		cm.Data(graphics::CubeMap::TOP, *srf);
	} catch (...) {
		SDL_FreeSurface(srf);
		throw;
	}
	SDL_FreeSurface(srf);

	if (!(srf = IMG_Load(bottom.c_str()))) throw SDLError("IMG_Load");
	try {
		cm.Data(graphics::CubeMap::BOTTOM, *srf);
	} catch (...) {
		SDL_FreeSurface(srf);
		throw;
	}
	SDL_FreeSurface(srf);

	if (!(srf = IMG_Load(back.c_str()))) throw SDLError("IMG_Load");
	try {
		cm.Data(graphics::CubeMap::BACK, *srf);
	} catch (...) {
		SDL_FreeSurface(srf);
		throw;
	}
	SDL_FreeSurface(srf);

	if (!(srf = IMG_Load(front.c_str()))) throw SDLError("IMG_Load");
	try {
		cm.Data(graphics::CubeMap::FRONT, *srf);
	} catch (...) {
		SDL_FreeSurface(srf);
		throw;
	}
	SDL_FreeSurface(srf);
}

void Assets::LoadUniverse(const string &name, world::Simulation &sim) const {
	std::ifstream universe_file(data_path + name);
	io::TokenStreamReader universe_reader(universe_file);
	ReadBody(universe_reader, sim);
	universe_reader.Skip(io::Token::SEMICOLON);
}

world::Body *Assets::ReadBody(io::TokenStreamReader &in, world::Simulation &sim) const {
	std::unique_ptr<world::Body> body;
	string name;
	in.ReadIdentifier(name);
	if (name == "Sun") {
		world::Sun *sun = new world::Sun;
		body.reset(sun);
		sim.AddSun(*sun);
		in.Skip(io::Token::ANGLE_BRACKET_OPEN);
		while (in.Peek().type != io::Token::ANGLE_BRACKET_CLOSE) {
			in.ReadIdentifier(name);
			in.Skip(io::Token::EQUALS);
			ReadSunProperty(name, in, *sun, sim);
			in.Skip(io::Token::SEMICOLON);
		}
		in.Skip(io::Token::ANGLE_BRACKET_CLOSE);
		in.Skip(io::Token::SEMICOLON);
	} else if (name == "Planet") {
		in.Skip(io::Token::PARENTHESIS_OPEN);
		int sidelength = in.GetInt();
		in.Skip(io::Token::PARENTHESIS_CLOSE);
		world::Planet *planet = new world::Planet(sidelength);
		sim.AddPlanet(*planet);
		body.reset(planet);
		in.Skip(io::Token::ANGLE_BRACKET_OPEN);
		while (in.Peek().type != io::Token::ANGLE_BRACKET_CLOSE) {
			in.ReadIdentifier(name);
			in.Skip(io::Token::EQUALS);
			ReadPlanetProperty(name, in, *planet, sim);
			in.Skip(io::Token::SEMICOLON);
		}
		in.Skip(io::Token::ANGLE_BRACKET_CLOSE);
	} else {
		throw std::runtime_error("unknown body class " + name);
	}
	return body.release();
}

void Assets::ReadSunProperty(const std::string &name, io::TokenStreamReader &in, world::Sun &sun, world::Simulation &sim) const {
	if (name == "color") {
		glm::dvec3 color(0.0);
		in.ReadVec(color);
		sun.Color(color);
	} else if (name == "luminosity") {
		sun.Luminosity(in.GetDouble());
	} else {
		ReadBodyProperty(name, in, sun, sim);
	}
}

void Assets::ReadPlanetProperty(const std::string &name, io::TokenStreamReader &in, world::Planet &planet, world::Simulation &sim) const {
	if (name == "generate") {
		string gen;
		in.ReadIdentifier(gen);
		if (gen == "earthlike") {
			world::GenerateEarthlike(data.tile_types, planet);
		} else if (gen == "test") {
			world::GenerateTest(data.tile_types, planet);
		} else {
			throw std::runtime_error("unknown surface generator " + gen);
		}
	} else if (name == "atmosphere") {
		string atm;
		in.ReadIdentifier(atm);
		planet.Atmosphere(data.resources[atm].id);
	} else {
		ReadBodyProperty(name, in, planet, sim);
	}
}

void Assets::ReadBodyProperty(const std::string &name, io::TokenStreamReader &in, world::Body &body, world::Simulation &sim) const {
	if (name == "name") {
		string value;
		in.ReadString(value);
		body.Name(value);
	} else if (name == "mass") {
		body.Mass(in.GetDouble());
	} else if (name == "radius") {
		body.Radius(in.GetDouble());
	} else if (name == "axial_tilt") {
		glm::dvec2 tilt(0.0);
		in.ReadVec(tilt);
		body.AxialTilt(tilt);
	} else if (name == "rotation") {
		body.Rotation(in.GetDouble());
	} else if (name == "angular_momentum") {
		body.AngularMomentum(in.GetDouble());
	} else if (name == "orbit") {
		in.Skip(io::Token::ANGLE_BRACKET_OPEN);
		while (in.Peek().type != io::Token::ANGLE_BRACKET_CLOSE) {
			string oname;
			in.ReadIdentifier(oname);
			in.Skip(io::Token::EQUALS);
			if (oname == "SMA" || oname == "semi_major_axis") {
				body.GetOrbit().SemiMajorAxis(in.GetDouble());
			} else if (oname == "ECC" || oname == "eccentricity") {
				body.GetOrbit().Eccentricity(in.GetDouble());
			} else if (oname == "INC" || oname == "inclination") {
				body.GetOrbit().Inclination(in.GetDouble());
			} else if (oname == "ASC" || oname == "ascending_node" || oname == "longitude_ascending") {
				body.GetOrbit().LongitudeAscending(in.GetDouble());
			} else if (oname == "ARG" || oname == "APE" || oname == "argument_periapsis") {
				body.GetOrbit().ArgumentPeriapsis(in.GetDouble());
			} else if (oname == "MNA" || oname == "mean_anomaly") {
				body.GetOrbit().MeanAnomaly(in.GetDouble());
			} else {
				throw std::runtime_error("unknown orbit property " + oname);
			}
			in.Skip(io::Token::SEMICOLON);
		}
		in.Skip(io::Token::ANGLE_BRACKET_CLOSE);
	} else if (name == "children") {
		in.Skip(io::Token::BRACKET_OPEN);
		while (in.Peek().type != io::Token::BRACKET_CLOSE) {
			world::Body *b = ReadBody(in, sim);
			b->SetParent(body);
			if (in.Peek().type == io::Token::COMMA) {
				in.Skip(io::Token::COMMA);
			}
		}
		in.Skip(io::Token::BRACKET_CLOSE);
	} else {
		throw std::runtime_error("unknown body property " + name);
	}
}

}
}
