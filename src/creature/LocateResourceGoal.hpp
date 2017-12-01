#ifndef BLOBS_CREATURE_LOCATERESOURCEGOAL_HPP_
#define BLOBS_CREATURE_LOCATERESOURCEGOAL_HPP_

#include "Composition.hpp"
#include "../math/glm.hpp"


namespace blobs {
namespace creature {

class LocateResourceGoal
: public Goal {

public:
	LocateResourceGoal(Creature &);
	~LocateResourceGoal() noexcept override;

public:
	void Accept(int resource, double attractiveness);

	std::string Describe() const override;
	void Enable() override;
	void Tick(double dt) override;
	void Action() override;

private:
	void LocateResource();
	void SearchVicinity();
	bool OnTargetTile() const noexcept;

private:
	Composition accept;
	bool found;
	glm::dvec3 target_pos;
	int target_srf;
	glm::ivec2 target_tile;
	bool searching;
	double reevaluate;

};

}
}

#endif
