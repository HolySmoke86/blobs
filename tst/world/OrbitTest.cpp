#include "OrbitTest.hpp"

#include "../assert.hpp"

#include "const.hpp"
#include "world/Orbit.hpp"

CPPUNIT_TEST_SUITE_REGISTRATION(blobs::test::world::OrbitTest);

using blobs::world::Orbit;


namespace blobs {
namespace test {
namespace world {

void OrbitTest::setUp() {
}

void OrbitTest::tearDown() {
}


void OrbitTest::testSMA() {
	Orbit orbit;
	orbit.SemiMajorAxis(1.0);
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
		"wrong semi-major axis on orbit",
		1.0, orbit.SemiMajorAxis(), std::numeric_limits<double>::epsilon()
	);

	// reference direction is +X, so at t=0, the body should be
	// at (sma,0,0) relative to its parent
	glm::vec4 pos(orbit.Matrix(0.0) * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
	AssertEqual(
		"wrong position at t=0",
		glm::vec3(1.0f, 0.0f, 0.0f),
		glm::vec3(pos) / pos.w
	);

	// at 90° position should be (0,0,sma) since the zero inclination
	// reference plane is XZ and rotates counter-clockwise
	pos = orbit.Matrix(PI_0p5) * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	AssertEqual(
		"wrong position at t=90°",
		glm::vec3(0.0f, 0.0f, -1.0f),
		glm::vec3(pos) / pos.w
	);

	// at 180° position should be (-sma,0,0)
	pos = orbit.Matrix(PI) * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	AssertEqual(
		"wrong position at t=180°",
		glm::vec3(-1.0f, 0.0f, 0.0f),
		glm::vec3(pos) / pos.w
	);

	// at 270° position should be (0,0,-sma)
	pos = orbit.Matrix(PI_1p5) * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	AssertEqual(
		"wrong position at t=270°",
		glm::vec3(0.0f, 0.0f, 1.0f),
		glm::vec3(pos) / pos.w
	);

	// at 360° position should be (sma,0,0), the initial position
	pos = orbit.Matrix(PI_2p0) * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	AssertEqual(
		"wrong position at t=360°",
		glm::vec3(1.0f, 0.0f, 0.0f),
		glm::vec3(pos) / pos.w
	);
}

}
}
}
