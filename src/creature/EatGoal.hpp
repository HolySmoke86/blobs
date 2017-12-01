#ifndef BLOBS_CREATURE_EATGOAL_HPP_
#define BLOBS_CREATURE_EATGOAL_HPP_

#include "Goal.hpp"


namespace blobs {
namespace creature {

class EatGoal
: public Goal {

public:
	explicit EatGoal(Creature &);
	~EatGoal() override;

public:
	std::string Describe() const override;
	void Enable() override;
	void Tick(double dt) override;
	void Action() override;

private:
	bool OnSuitableTile() const;

private:
	Goal *locate_subtask;
	bool eating;

};

}
}

#endif
