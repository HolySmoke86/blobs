#ifndef BLOBS_CREATURE_NEED_HPP_
#define BLOBS_CREATURE_NEED_HPP_

namespace blobs {
namespace creature {

struct Need {

	int resource = -1;
	double value = 0.0;

	// how fast value grows per second
	double gain = 0.0;
	// the value at which this need is no longer satisfied
	double critical = 0.0;
	// the price to pay for not satsfying the need
	double penalty = 0.0;

	void Tick(double dt) noexcept;

	bool IsSatisfied() const noexcept { return value < critical; }

};

}
}

#endif
