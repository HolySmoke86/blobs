#ifndef BLOBS_WORLD_RECORD_HPP_
#define BLOBS_WORLD_RECORD_HPP_

#include <string>


namespace blobs {
namespace creature {
	class Creature;
}
namespace world {

struct Record {

	static constexpr int MAX = 10;

	std::string name = "";
	enum Type {
		VALUE,
		LENGTH,
		MASS,
		PERCENTAGE,
		TIME,
	} type = VALUE;
	struct Rank {
		creature::Creature *holder = nullptr;
		double value = 0.0;
		double time = 0.0;
		operator bool() const noexcept { return holder; }
	} rank[MAX];

	operator bool() const noexcept { return rank[0]; }

	Rank *begin() noexcept { return rank; }
	const Rank *begin() const noexcept { return rank; }
	const Rank *cbegin() const noexcept { return rank; }

	Rank *end() noexcept { return rank + 10; }
	const Rank *end() const noexcept { return rank + 10; }
	const Rank *cend() const noexcept { return rank + 10; }

	/// update hiscore table, returns rank of given creature or -1 if not ranked
	int Update(creature::Creature &, double value, double time) noexcept;

	std::string ValueString(int i) const;

};

}
}

#endif
