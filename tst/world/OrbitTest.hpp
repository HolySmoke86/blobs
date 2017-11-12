#ifndef BLOBS_TEST_WORLD_ORBITTEST_H_
#define BLOBS_TEST_WORLD_ORBITTEST_H_

#include <cppunit/extensions/HelperMacros.h>


namespace blobs {
namespace test {
namespace world {

class OrbitTest
: public CppUnit::TestFixture {

CPPUNIT_TEST_SUITE(OrbitTest);

CPPUNIT_TEST(testDefaultOrbit);

CPPUNIT_TEST(testSMA);
CPPUNIT_TEST(testEcc);
CPPUNIT_TEST(testInc);
CPPUNIT_TEST(testLngAsc);
CPPUNIT_TEST(testArgPe);
CPPUNIT_TEST(testMnAn);

CPPUNIT_TEST_SUITE_END();

public:
	void setUp();
	void tearDown();

	void testDefaultOrbit();

	void testSMA();
	void testEcc();
	void testInc();
	void testLngAsc();
	void testArgPe();
	void testMnAn();

};

}
}
}

#endif
