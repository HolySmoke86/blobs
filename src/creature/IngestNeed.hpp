#ifndef BLOBS_CREATURE_INGESTNEED_HPP_
#define BLOBS_CREATURE_INGESTNEED_HPP_

#include "Need.hpp"


namespace blobs {
namespace creature {

class Need;

class IngestNeed
: public Need {

public:
	IngestNeed(int resource, double speed, double damage);
	~IngestNeed() override;

public:
	void ApplyEffect(Creature &, double dt) override;

private:
	int resource;
	double speed;
	double damage;
	bool ingesting;
	bool locating;

};

}
}

#endif
