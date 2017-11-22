#ifndef BLOBS_CREATURE_NEED_HPP_
#define BLOBS_CREATURE_NEED_HPP_

#include <string>


namespace blobs {
namespace creature {

class Creature;
class Effect;

struct Need {

	std::string name;

	double value = 0.0;

	// how fast value grows per second
	double gain = 0.0;
	// the value at which this need is no longer satisfied
	double inconvenient = 0.0;
	// the value at which this need starts to hurt
	double critical = 0.0;

	virtual ~Need() noexcept;

	void Tick(double dt) noexcept;
	void Increase(double) noexcept;
	void Decrease(double) noexcept;

	bool IsSatisfied() const noexcept { return value < inconvenient; }
	bool IsInconvenient() const noexcept { return value >= inconvenient && value < critical; }
	bool IsCritical() const noexcept { return value >= critical; }

	virtual void ApplyEffect(Creature &, double dt) = 0;

};

}
}

#endif
