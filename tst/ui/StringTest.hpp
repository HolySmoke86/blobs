#ifndef BLOBS_TEST_UI_STRINGTEST_HPP_
#define BLOBS_TEST_UI_STRINGTEST_HPP_

#include <cppunit/extensions/HelperMacros.h>


namespace blobs {
namespace ui {
namespace test {

class StringTest
: public CppUnit::TestFixture {

CPPUNIT_TEST_SUITE(StringTest);

CPPUNIT_TEST(testAngle);
CPPUNIT_TEST(testDecimal);
CPPUNIT_TEST(testLength);
CPPUNIT_TEST(testMass);
CPPUNIT_TEST(testNumber);
CPPUNIT_TEST(testPercentage);
CPPUNIT_TEST(testTime);
CPPUNIT_TEST(testVector);

CPPUNIT_TEST_SUITE_END();

public:
	void setUp();
	void tearDown();

	void testAngle();
	void testDecimal();
	void testLength();
	void testMass();
	void testNumber();
	void testPercentage();
	void testTime();
	void testVector();

};

}
}
}

#endif
