#ifndef BLOBS_TEST_GRAPHICS_COLORTEST_HPP
#define BLOBS_TEST_GRAPHICS_COLORTEST_HPP

#include <cppunit/extensions/HelperMacros.h>



namespace blobs {
namespace graphics {
namespace test {

class ColorTest
: public CppUnit::TestFixture {

CPPUNIT_TEST_SUITE(ColorTest);

CPPUNIT_TEST(testConversion);

CPPUNIT_TEST_SUITE_END();

public:
	void setUp();
	void tearDown();

	void testConversion();

};

}
}
}

#endif
