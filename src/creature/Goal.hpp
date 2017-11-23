#ifndef BLOBS_CREATURE_GOAL_HPP_
#define BLOBS_CREATURE_GOAL_HPP_

namespace blobs {
namespace creature {

class Creature;

class Goal {

public:
	Goal();
	virtual ~Goal() noexcept;

public:
	double Urgency() const noexcept { return urgency; }
	void Urgency(double u) noexcept { urgency = u; }

	bool Interruptible() const noexcept { return interruptible; }
	void Interruptible(bool i) noexcept { interruptible = i; }

	bool Complete() const noexcept { return complete; }
	void Complete(bool i) noexcept { complete = i; }

public:
	virtual void Enable(Creature &) { }
	virtual void Tick(double dt) { }
	virtual void Action(Creature &) { }

private:
	double urgency;
	bool interruptible;
	bool complete;

};

}
}

#endif
