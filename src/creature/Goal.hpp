#ifndef BLOBS_CREATURE_GOAL_HPP_
#define BLOBS_CREATURE_GOAL_HPP_

#include "Creature.hpp"

#include <functional>
#include <string>


namespace blobs {
namespace app {
	struct Assets;
}
namespace math {
	class GaloisLFSR;
}
namespace creature {

class Goal {

public:
	using Callback = std::function<void(Goal &)>;

public:
	explicit Goal(Creature &);
	virtual ~Goal() noexcept;

public:
	Creature &GetCreature() noexcept { return c; }
	const Creature &GetCreature() const noexcept { return c; }
	Creature::Stats &GetStats() noexcept { return c.GetStats(); }
	const Creature::Stats &GetStats() const noexcept { return c.GetStats(); }
	Situation &GetSituation() noexcept { return c.GetSituation(); }
	const Situation &GetSituation() const noexcept { return c.GetSituation(); }
	Steering &GetSteering() noexcept { return c.GetSteering(); }
	const Steering &GetSteering() const noexcept { return c.GetSteering(); }
	app::Assets &Assets() noexcept;
	const app::Assets &Assets() const noexcept;
	math::GaloisLFSR &Random() noexcept;

	double Urgency() const noexcept { return urgency; }
	void Urgency(double u) noexcept { urgency = u; }

	bool Interruptible() const noexcept { return interruptible; }
	void Interruptible(bool i) noexcept { interruptible = i; }

	bool Complete() const noexcept { return complete; }
	void SetComplete();
	void SetForeground();
	void SetBackground();
	/// only supports one callback for now, new one will replace an old
	void WhenComplete(Callback) noexcept;
	void WhenForeground(Callback) noexcept;
	/// on background will not be called when the goal is first inserted
	/// but only after is has been in the foreground once
	void WhenBackground(Callback) noexcept;

public:
	virtual std::string Describe() const = 0;
	virtual void Enable() { }
	virtual void Tick(double dt) { }
	virtual void Action() { }

private:
	virtual void OnComplete() { }
	virtual void OnForeground() { }
	virtual void OnBackground() { }

private:
	Creature &c;
	Callback on_complete;
	Callback on_foreground;
	Callback on_background;
	double urgency;
	bool interruptible;
	bool complete;

};

}
}

#endif
