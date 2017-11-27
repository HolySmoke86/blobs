#ifndef BLOBS_CREATURE_NAMEGENERATOR_HPP_
#define BLOBS_CREATURE_NAMEGENERATOR_HPP_

#include <string>


namespace blobs {
namespace creature {

class NameGenerator {

public:
	NameGenerator();
	~NameGenerator();

public:
	std::string Sequential();

private:
	int counter;

};

}
}

#endif
