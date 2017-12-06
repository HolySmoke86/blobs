#ifndef BLOBS_CREATURE_STROLLGOAL_HPP_
#define BLOBS_CREATURE_STROLLGOAL_HPP_

#include "Goal.hpp"

#include "../math/glm.hpp"


namespace blobs {
namespace creature {

class StrollGoal
: public Goal {

public:
	explicit StrollGoal(Creature &);
	~StrollGoal() override;

public:
	std::string Describe() const override;
	void Enable() override;
	void Action() override;
	void OnBackground() override;

	void PickTarget() noexcept;

private:
	glm::dvec3 last;
	glm::dvec3 next;

};

}
}

#endif
