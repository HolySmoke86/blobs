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

	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
		"wrong scalar conversion",
		-2.5, p.TileToPosition(0), std::numeric_limits<double>::epsilon());
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
		"wrong scalar conversion",
		-0.5, p.TileToPosition(2), std::numeric_limits<double>::epsilon());
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"wrong scalar conversion",
		2, p.PositionToTile(0.1));
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"wrong scalar conversion",
		2, p.PositionToTile(-0.1));
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"wrong scalar conversion",
		1, p.PositionToTile(-0.6));
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"wrong scalar conversion",
		4, p.PositionToTile(2.0));

	AssertEqual(
		"wrong surface position",
		glm::ivec2(2, 2), p.SurfacePosition(0, glm::dvec3(0.0, 0.0, 2.5))
	);
	AssertEqual(
		"wrong surface position",
		glm::ivec2(2, 2), p.SurfacePosition(0, glm::dvec3(0.1, 0.1, 2.5))
	);
	AssertEqual(
		"wrong surface position",
		glm::ivec2(2, 2), p.SurfacePosition(0, glm::dvec3(-0.1, -0.1, 2.5))
	);
	AssertEqual(
		"wrong surface position",
		glm::ivec2(3, 1), p.SurfacePosition(0, glm::dvec3(1.0, -1.0, 2.5))
	);

	AssertEqual(
		"wrong tile center",
		glm::dvec3(0.0, 0.0, 2.5), p.TileCenter(0, 2, 2)
	);
}

}
}
}
