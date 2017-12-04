#include "Record.hpp"
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

std::string Record::ValueString() const {
	switch (type) {
		default:
		case VALUE:
			return ui::DecimalString(value, 2);
		case LENGTH:
			return ui::LengthString(value);
		case MASS:
			return ui::MassString(value);
		case PERCENTAGE:
			return ui::PercentageString(value);
		case TIME:
			return ui::TimeString(value);
	}
}

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
		Record rold(records[0]);
		records[0].value = c.Age();
		records[0].time = Time();
		records[0].holder = &c;
		if (rold.holder && rold.holder != &c) {
			LogRecord(rold, records[0]);
		}
	}
	if (c.Mass() > records[1].value) {
		Record rold(records[1]);
		records[1].value = c.Mass();
		records[1].time = Time();
		records[1].holder = &c;
		if (rold.holder && rold.holder != &c) {
			LogRecord(rold, records[1]);
		}
	}
	if (c.Size() > records[2].value) {
		Record rold(records[2]);
		records[2].value = c.Size();
		records[2].time = Time();
		records[2].holder = &c;
		if (rold.holder && rold.holder != &c) {
			LogRecord(rold, records[2]);
		}
	}
	if (c.Strength() > records[3].value) {
		Record rold(records[3]);
		records[3].value = c.Strength();
		records[3].time = Time();
		records[3].holder = &c;
		if (rold.holder && rold.holder != &c) {
			LogRecord(rold, records[3]);
		}
	}
	if (c.Stamina() > records[4].value) {
		Record rold(records[4]);
		records[4].value = c.Stamina();
		records[4].time = Time();
		records[4].holder = &c;
		if (rold.holder && rold.holder != &c) {
			LogRecord(rold, records[4]);
		}
	}
	if (c.Dexerty() > records[5].value) {
		Record rold(records[5]);
		records[5].value = c.Dexerty();
		records[5].time = Time();
		records[5].holder = &c;
		if (rold.holder && rold.holder != &c) {
			LogRecord(rold, records[5]);
		}
	}
	if (c.Intelligence() > records[6].value) {
		Record rold(records[6]);
		records[6].value = c.Intelligence();
		records[6].time = Time();
		records[6].holder = &c;
		if (rold.holder && rold.holder != &c) {
			LogRecord(rold, records[6]);
		}
	}
}

void Simulation::LogRecord(const Record &rold, const Record &rnew) {
	Log() << "at age " << ui::TimeString(rnew.holder->Age()) << " "
		<< rnew.holder->Name() << " broke the " << rnew.name << " record of "
		<< rold.ValueString() << " by " << rold.holder->Name()
		<< " (established " << ui::TimeString(rold.time) << ")" << std::endl;
}

std::ostream &Simulation::Log() {
	return std::cout << '[' << ui::TimeString(Time()) << "] ";
}

}
}
