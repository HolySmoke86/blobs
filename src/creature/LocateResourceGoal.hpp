#ifndef BLOBS_CREATURE_LOCATERESOURCEGOAL_HPP_
#define BLOBS_CREATURE_LOCATERESOURCEGOAL_HPP_

namespace blobs {
namespace creature {

class LocateResourceGoal
: public Goal {

public:
	explicit LocateResourceGoal(int resource);
	~LocateResourceGoal() noexcept override;

public:
	void Enable(Creature &) override;
	void Tick(double dt) override;
	void Action(Creature &) override;

private:
	int res;

};

}
}

#endif
