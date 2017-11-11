#ifndef BLOBS_TEST_WORLD_ORBITTEST_H_
#define BLOBS_TEST_WORLD_ORBITTEST_H_

#include <cppunit/extensions/HelperMacros.h>


namespace blobs {
namespace test {
namespace world {

class OrbitTest
: public CppUnit::TestFixture {

CPPUNIT_TEST_SUITE(OrbitTest);

CPPUNIT_TEST(testSMA);

CPPUNIT_TEST_SUITE_END();

public:
	void setUp();
	void tearDown();

	void testSMA();

};

}
}
}

#endif
