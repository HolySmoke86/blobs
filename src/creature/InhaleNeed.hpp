#ifndef BLOBS_CREATURE_INHALENEED_HPP_
#define BLOBS_CREATURE_INHALENEED_HPP_

#include "Need.hpp"


namespace blobs {
namespace creature {

class Need;

class InhaleNeed
: public Need {

public:
	InhaleNeed(int resource, double speed, double damage);
	~InhaleNeed() override;

public:
	void ApplyEffect(Creature &, double dt) override;

private:
	int resource;
	double speed;
	double damage;
	bool inhaling;

};

}
}

#endif
