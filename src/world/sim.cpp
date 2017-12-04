#include "Simulation.hpp"

#include "Body.hpp"
#include "Planet.hpp"
#include "Sun.hpp"
#include "../creature/Creature.hpp"
#include "../ui/string.hpp"

#include <algorithm>
#include <iostream>


namespace blobs {
namespace world {

Simulation::Simulation(Body &r, app::Assets &assets)
: root(r)
, assets(assets)
, bodies()
, planets()
, suns()
, alive()
, dead()
, time(0.0)
, records(7) {
	AddBody(r);
	records[0].name = "Age";
	records[0].type = Record::TIME;
	records[1].name = "Mass";
	records[1].type = Record::MASS;
	records[2].name = "Size";
	records[2].type = Record::LENGTH;
	records[3].name = "Strength";
	records[4].name = "Stamina";
	records[5].name = "Dexerty";
	records[6].name = "Intelligence";
}

Simulation::~Simulation() {
	for (auto c : alive) {
		delete c;
	}
	for (auto c : dead) {
		delete c;
	}
}

void Simulation::Tick(double dt) {
	time += dt;
	for (auto body : bodies) {
		body->Tick(dt);
	}
	for (auto c : alive) {
		CheckRecords(*c);
	}
}

void Simulation::AddBody(Body &b) {
	b.SetSimulation(*this);
	bodies.insert(&b);
}

void Simulation::AddPlanet(Planet &p) {
	AddBody(p);
	planets.insert(&p);
}

void Simulation::AddSun(Sun &s) {
	AddBody(s);
	suns.insert(&s);
}

void Simulation::SetAlive(creature::Creature *c) {
	alive.push_back(c);
}

void Simulation::SetDead(creature::Creature *c) {
	auto entry = std::find(alive.begin(), alive.end(), c);
	if (entry != alive.end()) {
		alive.erase(entry);
	}
	dead.push_back(c);
	CheckRecords(*c);
}

void Simulation::CheckRecords(creature::Creature &c) noexcept {
	if (c.Age() > records[0].value) {
		if (records[0].holder && records[0].holder != &c) {
			Log() << "new age record by " << c.Name() << std::endl;
		}
		records[0].value = c.Age();
		records[0].time = Time();
		records[0].holder = &c;
	}
	if (c.Mass() > records[1].value) {
		if (records[1].holder && records[1].holder != &c) {
			Log() << "new mass record by " << c.Name() << std::endl;
		}
		records[1].value = c.Mass();
		records[1].time = Time();
		records[1].holder = &c;
	}
	if (c.Size() > records[2].value) {
		if (records[2].holder && records[2].holder != &c) {
			Log() << "new size record by " << c.Name() << std::endl;
		}
		records[2].value = c.Size();
		records[2].time = Time();
		records[2].holder = &c;
	}
	if (c.Strength() > records[3].value) {
		if (records[3].holder && records[3].holder != &c) {
			Log() << "new strength record by " << c.Name() << std::endl;
		}
		records[3].value = c.Strength();
		records[3].time = Time();
		records[3].holder = &c;
	}
	if (c.Stamina() > records[4].value) {
		if (records[4].holder && records[4].holder != &c) {
			Log() << "new stamina record by " << c.Name() << std::endl;
		}
		records[4].value = c.Stamina();
		records[4].time = Time();
		records[4].holder = &c;
	}
	if (c.Dexerty() > records[5].value) {
		if (records[5].holder && records[5].holder != &c) {
			Log() << "new dexerty record by " << c.Name() << std::endl;
		}
		records[5].value = c.Dexerty();
		records[5].time = Time();
		records[5].holder = &c;
	}
	if (c.Intelligence() > records[6].value) {
		if (records[6].holder && records[6].holder != &c) {
			Log() << "new intelligence record by " << c.Name() << std::endl;
		}
		records[6].value = c.Intelligence();
		records[6].time = Time();
		records[6].holder = &c;
	}
}

std::ostream &Simulation::Log() {
	return std::cout << '[' << ui::TimeString(Time()) << "] ";
}

}
}
