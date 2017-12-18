#ifndef BLOBS_CREATURE_LOOKAROUNDGOAL_HPP_
#define BLOBS_CREATURE_LOOKAROUNDGOAL_HPP_

#include "Goal.hpp"


namespace blobs {
namespace creature {

class LookAroundGoal
: public Goal {

public:
	explicit LookAroundGoal(Creature &);
	~LookAroundGoal() override;

public:
	std::string Describe() const override;
	void Enable() override;
	void Tick(double dt) override;
	void Action() override;
	void OnBackground() override;

	void PickDirection() noexcept;

private:
	double timer;

};

}
}

#endif
