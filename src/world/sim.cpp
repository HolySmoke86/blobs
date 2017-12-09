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

int Record::Update(creature::Creature &c, double value, double time) noexcept {
	int found = -1;
	for (int i = 0; i < MAX; ++i) {
		if (value > rank[i].value) {
			found = i;
			break;
		}
	}
	if (found < 0) {
		return -1;
	}
	int previous = -1;
	for (int i = 0; i < MAX; ++i) {
		if (rank[i].holder == &c) {
			previous = i;
			break;
		}
	}
	if (previous < 0) {
		// move all below down by one
		std::copy_backward(rank + found, rank + MAX - 1, rank + MAX);
	} else if (found > previous) {
		// better than last, but not an improvement
		// this ensures only one slot occupied per creature
		return found;
	} else if (found < previous) {
		// move all in between down by one
		std::copy_backward(rank + found, rank + previous, rank + previous + 1);
	}
	// insert new
	rank[found].holder = &c;
	rank[found].value = value;
	rank[found].time = time;
	return found;
}

std::string Record::ValueString(int i) const {
	if (i < 0 || i >= MAX || !rank[i].holder) {
		return "—";
	}
	switch (type) {
		default:
		case VALUE:
			return ui::DecimalString(rank[i].value, 2);
		case LENGTH:
			return ui::LengthString(rank[i].value);
		case MASS:
			return ui::MassString(rank[i].value);
		case PERCENTAGE:
			return ui::PercentageString(rank[i].value);
		case TIME:
			return ui::TimeString(rank[i].value);
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
	{ // age
		creature::Creature *prev = records[0].rank[0].holder;
		int rank = records[0].Update(c, c.Age(), time);
		if (rank == 0 && prev && prev != &c) {
			LogRecord(records[0]);
		}
	}
	{ // mass
		creature::Creature *prev = records[1].rank[0].holder;
		int rank = records[1].Update(c, c.Mass(), time);
		if (rank == 0 && prev && prev != &c) {
			LogRecord(records[1]);
		}
	}
	{ // size
		creature::Creature *prev = records[2].rank[0].holder;
		int rank = records[2].Update(c, c.Size(), time);
		if (rank == 0 && prev && prev != &c) {
			LogRecord(records[2]);
		}
	}
	{ // strength
		creature::Creature *prev = records[3].rank[0].holder;
		int rank = records[3].Update(c, c.Strength(), time);
		if (rank == 0 && prev && prev != &c) {
			LogRecord(records[3]);
		}
	}
	{ // stamina
		creature::Creature *prev = records[4].rank[0].holder;
		int rank = records[4].Update(c, c.Stamina(), time);
		if (rank == 0 && prev && prev != &c) {
			LogRecord(records[4]);
		}
	}
	{ // dexerty
		creature::Creature *prev = records[5].rank[0].holder;
		int rank = records[5].Update(c, c.Dexerty(), time);
		if (rank == 0 && prev && prev != &c) {
			LogRecord(records[5]);
		}
	}
	{ // intelligence
		creature::Creature *prev = records[6].rank[0].holder;
		int rank = records[6].Update(c, c.Intelligence(), time);
		if (rank == 0 && prev && prev != &c) {
			LogRecord(records[6]);
		}
	}
}

void Simulation::LogRecord(const Record &r) {
	Log() << "at age " << ui::TimeString(r.rank[0].holder->Age()) << " "
		<< r.rank[0].holder->Name() << " broke the " << r.name << " record of "
		<< r.ValueString(1) << " by " << r.rank[1].holder->Name()
		<< " (established " << ui::TimeString(r.rank[1].time) << ")" << std::endl;
}

std::ostream &Simulation::Log() {
	return std::cout << '[' << ui::TimeString(Time()) << "] ";
}

}
}
