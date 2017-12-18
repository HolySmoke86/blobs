#ifndef BLOBS_TEST_APP_MASTERTEST_HPP_
#define BLOBS_TEST_APP_MASTERTEST_HPP_

#include <cppunit/extensions/HelperMacros.h>


namespace blobs {
namespace app {
namespace test {

class MasterTest
: public CppUnit::TestFixture {

CPPUNIT_TEST_SUITE(MasterTest);

CPPUNIT_TEST(testOneSecond);
CPPUNIT_TEST(testBasicInteraction);

CPPUNIT_TEST_SUITE_END();

public:
	void setUp();
	void tearDown();

	void testOneSecond();
	void testBasicInteraction();

};

}
}
}

#endif
