#ifndef BLOBS_CREATURE_INGESTGOAL_HPP_
#define BLOBS_CREATURE_INGESTGOAL_HPP_

#include "Goal.hpp"

#include "Composition.hpp"
#include "Creature.hpp"


namespace blobs {
namespace creature {

class LocateResourceGoal;

class IngestGoal
: public Goal {

public:
	explicit IngestGoal(Creature &, Creature::Stat &);
	~IngestGoal() override;

public:
	void Accept(int resource, double value);

	std::string Describe() const override;
	void Enable() override;
	void Tick(double dt) override;
	void Action() override;

private:
	bool OnSuitableTile();

private:
	Creature::Stat &stat;
	Composition accept;
	LocateResourceGoal *locate_subtask;
	bool ingesting;

	int resource;
	double yield;

};

}
}

#endif
