#ifndef BLOBS_CREATURE_BLOBBACKGROUNDTASK_HPP_
#define BLOBS_CREATURE_BLOBBACKGROUNDTASK_HPP_

#include "Goal.hpp"


namespace blobs {
namespace creature {

class IngestGoal;

class BlobBackgroundTask
: public Goal {

public:
	explicit BlobBackgroundTask(Creature &);
	~BlobBackgroundTask() override;

public:
	std::string Describe() const override;
	void Tick(double dt) override;
	void Action() override;

private:
	void CheckStats();
	void CheckSplit();
	void CheckMutate();

private:
	bool breathing;
	IngestGoal *drink_subtask;
	IngestGoal *eat_subtask;

};

}
}

#endif
