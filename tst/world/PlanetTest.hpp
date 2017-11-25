#ifndef BLOBS_TEST_WORLD_PLANETTEST_HPP_
#define BLOBS_TEST_WORLD_PLANETTEST_HPP_

#include <cppunit/extensions/HelperMacros.h>


namespace blobs {
namespace world {
namespace test {

class PlanetTest
: public CppUnit::TestFixture {

CPPUNIT_TEST_SUITE(PlanetTest);

CPPUNIT_TEST(testPositionConversion);

CPPUNIT_TEST_SUITE_END();

public:
	void setUp();
	void tearDown();

	void testPositionConversion();

};

}
}
}

#endif
