#ifndef BLOBS_CREATURE_IDLEGOAL_HPP_
#define BLOBS_CREATURE_IDLEGOAL_HPP_

#include "Goal.hpp"


namespace blobs {
namespace creature {

class IdleGoal
: public Goal {

public:
	explicit IdleGoal(Creature &);
	~IdleGoal() override;

public:
	std::string Describe() const override;
	void Action() override;

	void PickActivity();

};

}
}

#endif
