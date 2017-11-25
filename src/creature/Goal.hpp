#ifndef BLOBS_CREATURE_GOAL_HPP_
#define BLOBS_CREATURE_GOAL_HPP_

#include <functional>
#include <string>


namespace blobs {
namespace creature {

class Creature;
class Situation;
class Steering;

class Goal {

public:
	using Callback = std::function<void(Goal &)>;

public:
	explicit Goal(Creature &);
	virtual ~Goal() noexcept;

public:
	Creature &GetCreature() noexcept { return c; }
	const Creature &GetCreature() const noexcept { return c; }
	Situation &GetSituation() noexcept;
	const Situation &GetSituation() const noexcept;
	Steering &GetSteering() noexcept;
	const Steering &GetSteering() const noexcept;

	double Urgency() const noexcept { return urgency; }
	void Urgency(double u) noexcept { urgency = u; }

	bool Interruptible() const noexcept { return interruptible; }
	void Interruptible(bool i) noexcept { interruptible = i; }

	bool Complete() const noexcept { return complete; }
	void SetComplete() noexcept;
	/// only supports one callback for now, new one will replace an old
	void OnComplete(Callback) noexcept;

public:
	virtual std::string Describe() const = 0;
	virtual void Enable() { }
	virtual void Tick(double dt) { }
	virtual void Action() { }

private:
	Creature &c;
	Callback on_complete;
	double urgency;
	bool interruptible;
	bool complete;

};

}
}

#endif
