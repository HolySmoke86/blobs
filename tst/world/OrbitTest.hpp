#ifndef BLOBS_TEST_WORLD_ORBITTEST_H_
#define BLOBS_TEST_WORLD_ORBITTEST_H_

#include <cppunit/extensions/HelperMacros.h>


namespace blobs {
namespace world {
namespace test {

class OrbitTest
: public CppUnit::TestFixture {

CPPUNIT_TEST_SUITE(OrbitTest);

CPPUNIT_TEST(testDefault);
CPPUNIT_TEST(testSMA);
CPPUNIT_TEST(testEcc);
CPPUNIT_TEST(testInc);
CPPUNIT_TEST(testLngAsc);
CPPUNIT_TEST(testArgPe);
CPPUNIT_TEST(testMnAn);

CPPUNIT_TEST(testInverseDefault);
CPPUNIT_TEST(testInverseSMA);
CPPUNIT_TEST(testInverseEcc);
CPPUNIT_TEST(testInverseInc);
CPPUNIT_TEST(testInverseLngAsc);
CPPUNIT_TEST(testInverseArgPe);
CPPUNIT_TEST(testInverseMnAn);

CPPUNIT_TEST_SUITE_END();

public:
	void setUp();
	void tearDown();

	void testDefault();
	void testSMA();
	void testEcc();
	void testInc();
	void testLngAsc();
	void testArgPe();
	void testMnAn();

	void testInverseDefault();
	void testInverseSMA();
	void testInverseEcc();
	void testInverseInc();
	void testInverseLngAsc();
	void testInverseArgPe();
	void testInverseMnAn();

};

}
}
}

#endif
