#ifndef BLOBS_TEST_MATH_STABILITYTEST_HPP
#define BLOBS_TEST_MATH_STABILITYTEST_HPP

#include "math/glm.hpp"

#include <cppunit/extensions/HelperMacros.h>



namespace blobs {
namespace math {

class SimplexNoise;
class WorleyNoise;

namespace test {

class StabilityTest
: public CppUnit::TestFixture {

CPPUNIT_TEST_SUITE(StabilityTest);

CPPUNIT_TEST(testRNG);
CPPUNIT_TEST(testSimplex);
CPPUNIT_TEST(testWorley);

CPPUNIT_TEST_SUITE_END();

public:
	void setUp();
	void tearDown();

	void testRNG();
	void testSimplex();
	void testWorley();

	static void Assert(
		const SimplexNoise &noise,
		const glm::vec3 &position,
		float expected);

	static void Assert(
		const WorleyNoise &noise,
		const glm::vec3 &position,
		float expected);

};

}
}
}

#endif
