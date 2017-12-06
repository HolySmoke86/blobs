#include "PlanetTest.hpp"

#include "../assert.hpp"

#include "world/Planet.hpp"

#include <limits>

CPPUNIT_TEST_SUITE_REGISTRATION(blobs::world::test::PlanetTest);

using blobs::test::AssertEqual;


namespace blobs {
namespace world {
namespace test {

void PlanetTest::setUp() {
}

void PlanetTest::tearDown() {
}


void PlanetTest::testPositionConversion() {
	Planet p(5);

	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"wrong sidelength of planet",
		5, p.SideLength());
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
		"wrong radius of planet",
		2.5, p.Radius(), std::numeric_limits<double>::epsilon());
}

}
}
}
