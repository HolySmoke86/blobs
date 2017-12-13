#ifndef BLOBS_CREATURE_ATTACKGOAL_HPP_
#define BLOBS_CREATURE_ATTACKGOAL_HPP_

#include "Goal.hpp"

namespace blobs {
namespace creature {

class AttackGoal
: public Goal {

public:
	AttackGoal(Creature &self, Creature &target);
	~AttackGoal() override;

public:
	std::string Describe() const override;
	void Tick(double dt) override;
	void Action() override;
	void OnBackground() override;

	void SetDamageTarget(double t) noexcept { damage_target = t; }

private:
	Creature &target;
	double damage_target;
	double damage_dealt;
	double cooldown;

};

}
}

#endif
