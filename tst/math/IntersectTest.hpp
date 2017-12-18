#ifndef BLOBS_TEST_MATH_INTERSECTTEST_HPP_
#define BLOBS_TEST_MATH_INTERSECTTEST_HPP_

#include <cppunit/extensions/HelperMacros.h>


namespace blobs {
namespace math {
namespace test {

class IntersectTest
: public CppUnit::TestFixture {

CPPUNIT_TEST_SUITE(IntersectTest);

CPPUNIT_TEST(testRayBoxIntersection);
CPPUNIT_TEST(testBoxBoxIntersection);
CPPUNIT_TEST(testRaySphereIntersection);

CPPUNIT_TEST_SUITE_END();

public:
	void setUp();
	void tearDown();

	void testRayBoxIntersection();
	void testBoxBoxIntersection();
	void testRaySphereIntersection();

};

}
}
}

#endif
