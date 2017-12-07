#include "OrbitTest.hpp"

#include "../assert.hpp"

#include "math/const.hpp"
#include "world/Orbit.hpp"

CPPUNIT_TEST_SUITE_REGISTRATION(blobs::world::test::OrbitTest);

using blobs::test::AssertEqual;


namespace blobs {
namespace world {
namespace test {

void OrbitTest::setUp() {
}

void OrbitTest::tearDown() {
}


void OrbitTest::testDefault() {
	Orbit orbit;
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
		"wrong semi-major axis on default orbit",
		1.0, orbit.SemiMajorAxis(), std::numeric_limits<double>::epsilon()
	);
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
		"wrong eccentricity on default orbit",
		0.0, orbit.Eccentricity(), std::numeric_limits<double>::epsilon()
	);
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
		"wrong inclination on default orbit",
		0.0, orbit.Inclination(), std::numeric_limits<double>::epsilon()
	);
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
		"wrong longitude of ascending node on default orbit",
		0.0, orbit.LongitudeAscending(), std::numeric_limits<double>::epsilon()
	);
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
		"wrong argument of periapsis on default orbit",
		0.0, orbit.ArgumentPeriapsis(), std::numeric_limits<double>::epsilon()
	);
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
		"wrong mean anomaly on default orbit",
		0.0, orbit.MeanAnomaly(), std::numeric_limits<double>::epsilon()
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
	pos = orbit.Matrix(PI * 0.5) * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
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
	pos = orbit.Matrix(PI * 1.5) * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	AssertEqual(
		"wrong position at t=270°",
		glm::vec3(0.0f, 0.0f, 1.0f),
		glm::vec3(pos) / pos.w
	);

	// at 360° position should be (sma,0,0), the initial position
	pos = orbit.Matrix(PI * 2.0) * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	AssertEqual(
		"wrong position at t=360°",
		glm::vec3(1.0f, 0.0f, 0.0f),
		glm::vec3(pos) / pos.w
	);
}

void OrbitTest::testSMA() {
	Orbit orbit;
	orbit.SemiMajorAxis(2.0);
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
		"wrong semi-major axis on orbit",
		2.0, orbit.SemiMajorAxis(), std::numeric_limits<double>::epsilon()
	);

	// reference direction is +X, so at t=0, the body should be
	// at (sma,0,0) relative to its parent
	glm::vec4 pos(orbit.Matrix(0.0) * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
	AssertEqual(
		"wrong position at t=0",
		glm::vec3(2.0f, 0.0f, 0.0f),
		glm::vec3(pos) / pos.w
	);

	// at 90° position should be (0,0,sma) since the zero inclination
	// reference plane is XZ and rotates counter-clockwise
	pos = orbit.Matrix(PI * 0.5) * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	AssertEqual(
		"wrong position at t=90°",
		glm::vec3(0.0f, 0.0f, -2.0f),
		glm::vec3(pos) / pos.w
	);

	// at 180° position should be (-sma,0,0)
	pos = orbit.Matrix(PI) * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	AssertEqual(
		"wrong position at t=180°",
		glm::vec3(-2.0f, 0.0f, 0.0f),
		glm::vec3(pos) / pos.w
	);

	// at 270° position should be (0,0,-sma)
	pos = orbit.Matrix(PI * 1.5) * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	AssertEqual(
		"wrong position at t=270°",
		glm::vec3(0.0f, 0.0f, 2.0f),
		glm::vec3(pos) / pos.w
	);

	// at 360° position should be (sma,0,0), the initial position
	pos = orbit.Matrix(PI * 2.0) * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	AssertEqual(
		"wrong position at t=360°",
		glm::vec3(2.0f, 0.0f, 0.0f),
		glm::vec3(pos) / pos.w
	);
}

void OrbitTest::testEcc() {
	Orbit orbit;
	orbit.Eccentricity(0.5);
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
		"wrong eccentricity on orbit",
		0.5, orbit.Eccentricity(), std::numeric_limits<double>::epsilon()
	);

	glm::vec4 pos(orbit.Matrix(0.0) * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
	AssertEqual(
		"wrong position at t=0",
		glm::vec3(0.5f, 0.0f, 0.0f),
		glm::vec3(pos) / pos.w
	);

	pos = orbit.Matrix(PI * 0.5) * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	AssertEqual(
		"wrong position at t=90°",
		glm::vec3(-0.935130834579468f, 0.0f, -0.779740869998932f),
		glm::vec3(pos) / pos.w
	);

	pos = orbit.Matrix(PI) * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	AssertEqual(
		"wrong position at t=180°",
		glm::vec3(-1.5f, 0.0f, 0.0f),
		glm::vec3(pos) / pos.w
	);

	pos = orbit.Matrix(PI * 1.5) * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	AssertEqual(
		"wrong position at t=270°",
		glm::vec3(-0.935130834579468f, 0.0f, 0.779740869998932f),
		glm::vec3(pos) / pos.w
	);

	pos = orbit.Matrix(PI * 2.0) * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	AssertEqual(
		"wrong position at t=360°",
		glm::vec3(0.5f, 0.0f, 0.0f),
		glm::vec3(pos) / pos.w
	);
}

void OrbitTest::testInc() {
	Orbit orbit;
	orbit.Inclination(PI * 0.25); // 45°
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
		"wrong inclination on orbit",
		PI * 0.25, orbit.Inclination(), std::numeric_limits<double>::epsilon()
	);

	// inclination rotates counter clockwise around +X, so at t=0 should be
	// at (sma,0,0) relative to its parent
	glm::vec4 pos(orbit.Matrix(0.0) * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
	AssertEqual(
		"wrong position at t=0",
		glm::vec3(1.0f, 0.0f, 0.0f),
		glm::vec3(pos) / pos.w
	);

	pos = orbit.Matrix(PI * 0.5) * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	AssertEqual(
		"wrong position at t=90°",
		glm::vec3(0.0f, 0.70710676908493f, -0.70710676908493f),
		glm::vec3(pos) / pos.w
	);

	pos = orbit.Matrix(PI) * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	AssertEqual(
		"wrong position at t=180°",
		glm::vec3(-1.0f, 0.0f, 0.0f),
		glm::vec3(pos) / pos.w
	);

	pos = orbit.Matrix(PI * 1.5) * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	AssertEqual(
		"wrong position at t=270°",
		glm::vec3(0.0f, -0.70710676908493f, 0.70710676908493f),
		glm::vec3(pos) / pos.w
	);

	pos = orbit.Matrix(PI * 2.0) * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	AssertEqual(
		"wrong position at t=360°",
		glm::vec3(1.0f, 0.0f, 0.0f),
		glm::vec3(pos) / pos.w
	);
}

void OrbitTest::testLngAsc() {
	Orbit orbit;
	orbit.LongitudeAscending(PI * 0.25); // 45°
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
		"wrong longitude of ascending node on orbit",
		PI * 0.25, orbit.LongitudeAscending(), std::numeric_limits<double>::epsilon()
	);
	// using an inclination of 90° as well to make the rotation more apparent
	orbit.Inclination(PI * 0.5);

	// inclination rotates counter clockwise around +X, while LAN rotates it
	// around +Y, so at t=0 should be at (sma*sin(45°),0,-sma*cos(45°))
	glm::vec4 pos(orbit.Matrix(0.0) * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
	AssertEqual(
		"wrong position at t=0",
		glm::vec3(0.70710676908493f, 0.0f, -0.70710676908493f),
		glm::vec3(pos) / pos.w
	);

	pos = orbit.Matrix(PI * 0.5) * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	AssertEqual(
		"wrong position at t=90°",
		glm::vec3(0.0f, 1.0f, 0.0f),
		glm::vec3(pos) / pos.w
	);

	pos = orbit.Matrix(PI) * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	AssertEqual(
		"wrong position at t=180°",
		glm::vec3(-0.70710676908493f, 0.0f, 0.70710676908493f),
		glm::vec3(pos) / pos.w
	);

	pos = orbit.Matrix(PI * 1.5) * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	AssertEqual(
		"wrong position at t=270°",
		glm::vec3(0.0f, -1.0f, 0.0f),
		glm::vec3(pos) / pos.w
	);

	pos = orbit.Matrix(PI * 2.0) * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	AssertEqual(
		"wrong position at t=360°",
		glm::vec3(0.70710676908493f, 0.0f, -0.70710676908493f),
		glm::vec3(pos) / pos.w
	);
}

void OrbitTest::testArgPe() {
	Orbit orbit;
	orbit.ArgumentPeriapsis(PI * 0.25); // 45°
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
		"wrong argument of periapsis node on orbit",
		PI * 0.25, orbit.ArgumentPeriapsis(), std::numeric_limits<double>::epsilon()
	);
	// using an inclination of 90° as well to make the rotation more apparent
	orbit.Inclination(PI * 0.5);

	// inclination rotates counter clockwise around +X, while APe rotates it
	// around +Y in the rotated coordinate system, so at t=0 should be at
	// (sma*sin(45°),0,sma*cos(45°))
	glm::vec4 pos(orbit.Matrix(0.0) * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
	AssertEqual(
		"wrong position at t=0",
		glm::vec3(0.70710676908493f, 0.0f, 0.70710676908493f),
		glm::vec3(pos) / pos.w
	);

	pos = orbit.Matrix(PI * 0.5) * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	AssertEqual(
		"wrong position at t=90°",
		glm::vec3(0.0f, 1.0f, 0.0f),
		glm::vec3(pos) / pos.w
	);

	pos = orbit.Matrix(PI) * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	AssertEqual(
		"wrong position at t=180°",
		glm::vec3(-0.70710676908493f, 0.0f, -0.70710676908493f),
		glm::vec3(pos) / pos.w
	);

	pos = orbit.Matrix(PI * 1.5) * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	AssertEqual(
		"wrong position at t=270°",
		glm::vec3(0.0f, -1.0f, 0.0f),
		glm::vec3(pos) / pos.w
	);

	pos = orbit.Matrix(PI * 2.0) * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	AssertEqual(
		"wrong position at t=360°",
		glm::vec3(0.70710676908493f, 0.0f, 0.70710676908493f),
		glm::vec3(pos) / pos.w
	);
}

void OrbitTest::testMnAn() {
	Orbit orbit;
	orbit.MeanAnomaly(PI * 0.25); // 45°
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
		"wrong mean anomaly on default orbit",
		PI * 0.25, orbit.MeanAnomaly(), std::numeric_limits<double>::epsilon()
	);

	// mean anomaly just phase shifts the orbit
	glm::vec4 pos(orbit.Matrix(0.0) * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
	AssertEqual(
		"wrong position at t=0",
		glm::vec3(0.70710676908493f, 0.0f, -0.70710676908493f),
		glm::vec3(pos) / pos.w
	);

	// at 90° position should be (0,0,sma) since the zero inclination
	// reference plane is XZ and rotates counter-clockwise
	pos = orbit.Matrix(PI * 0.5) * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	AssertEqual(
		"wrong position at t=90°",
		glm::vec3(-0.70710676908493f, 0.0f, -0.70710676908493f),
		glm::vec3(pos) / pos.w
	);

	// at 180° position should be (-sma,0,0)
	pos = orbit.Matrix(PI) * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	AssertEqual(
		"wrong position at t=180°",
		glm::vec3(-0.70710676908493f, 0.0f, 0.70710676908493f),
		glm::vec3(pos) / pos.w
	);

	// at 270° position should be (0,0,-sma)
	pos = orbit.Matrix(PI * 1.5) * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	AssertEqual(
		"wrong position at t=270°",
		glm::vec3(0.70710676908493f, 0.0f, 0.70710676908493f),
		glm::vec3(pos) / pos.w
	);

	// at 360° position should be (sma,0,0), the initial position
	pos = orbit.Matrix(PI * 2.0) * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	AssertEqual(
		"wrong position at t=360°",
		glm::vec3(0.70710676908493f, 0.0f, -0.70710676908493f),
		glm::vec3(pos) / pos.w
	);
}

void OrbitTest::testInverseDefault() {
	Orbit orbit;

	// inverse matrix should project expected orbit position back to the origin
	glm::vec4 pos(orbit.InverseMatrix(0.0) * glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
	AssertEqual(
		"wrong position at t=0",
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(pos) / pos.w
	);

	// at 90° position should be (0,0,sma) since the zero inclination
	// reference plane is XZ and rotates counter-clockwise
	pos = orbit.InverseMatrix(PI * 0.5) * glm::vec4(0.0f, 0.0f, -1.0f, 1.0f);
	AssertEqual(
		"wrong position at t=90°",
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(pos) / pos.w
	);

	// at 180° position should be (-sma,0,0)
	pos = orbit.InverseMatrix(PI) * glm::vec4(-1.0f, 0.0f, 0.0f, 1.0f);
	AssertEqual(
		"wrong position at t=180°",
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(pos) / pos.w
	);

	// at 270° position should be (0,0,-sma)
	pos = orbit.InverseMatrix(PI * 1.5) * glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
	AssertEqual(
		"wrong position at t=270°",
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(pos) / pos.w
	);

	// at 360° position should be (sma,0,0), the initial position
	pos = orbit.InverseMatrix(PI * 2.0) * glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
	AssertEqual(
		"wrong position at t=360°",
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(pos) / pos.w
	);
}

void OrbitTest::testInverseSMA() {
	Orbit orbit;
	orbit.SemiMajorAxis(2.0);

	// reference direction is +X, so at t=0, the body should be
	// at (sma,0,0) relative to its parent
	glm::vec4 pos(orbit.InverseMatrix(0.0) * glm::vec4(2.0f, 0.0f, 0.0f, 1.0f));
	AssertEqual(
		"wrong position at t=0",
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(pos) / pos.w
	);

	// at 90° position should be (0,0,sma) since the zero inclination
	// reference plane is XZ and rotates counter-clockwise
	pos = orbit.InverseMatrix(PI * 0.5) * glm::vec4(0.0f, 0.0f, -2.0f, 1.0f);
	AssertEqual(
		"wrong position at t=90°",
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(pos) / pos.w
	);

	// at 180° position should be (-sma,0,0)
	pos = orbit.InverseMatrix(PI) * glm::vec4(-2.0f, 0.0f, 0.0f, 1.0f);
	AssertEqual(
		"wrong position at t=180°",
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(pos) / pos.w
	);

	// at 270° position should be (0,0,-sma)
	pos = orbit.InverseMatrix(PI * 1.5) * glm::vec4(0.0f, 0.0f, 2.0f, 1.0f);
	AssertEqual(
		"wrong position at t=270°",
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(pos) / pos.w
	);

	// at 360° position should be (sma,0,0), the initial position
	pos = orbit.InverseMatrix(PI * 2.0) * glm::vec4(2.0f, 0.0f, 0.0f, 1.0f);
	AssertEqual(
		"wrong position at t=360°",
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(pos) / pos.w
	);
}

void OrbitTest::testInverseEcc() {
	Orbit orbit;
	orbit.Eccentricity(0.5);

	glm::vec4 pos(orbit.InverseMatrix(0.0) * glm::vec4(0.5f, 0.0f, 0.0f, 1.0f));
	AssertEqual(
		"wrong position at t=0",
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(pos) / pos.w
	);

	pos = orbit.InverseMatrix(PI * 0.5) * glm::vec4(-0.935130834579468f, 0.0f, -0.779740869998932f, 1.0f);
	AssertEqual(
		"wrong position at t=90°",
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(pos) / pos.w
	);

	pos = orbit.InverseMatrix(PI) * glm::vec4(-1.5f, 0.0f, 0.0f, 1.0f);
	AssertEqual(
		"wrong position at t=180°",
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(pos) / pos.w
	);

	pos = orbit.InverseMatrix(PI * 1.5) * glm::vec4(-0.935130834579468f, 0.0f, 0.779740869998932f, 1.0f);
	AssertEqual(
		"wrong position at t=270°",
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(pos) / pos.w
	);

	pos = orbit.InverseMatrix(PI * 2.0) * glm::vec4(0.5f, 0.0f, 0.0f, 1.0f);
	AssertEqual(
		"wrong position at t=360°",
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(pos) / pos.w
	);
}

void OrbitTest::testInverseInc() {
	Orbit orbit;
	orbit.Inclination(PI * 0.25); // 45°

	// inclination rotates counter clockwise around +X, so at t=0 should be
	// at (sma,0,0) relative to its parent
	glm::vec4 pos(orbit.InverseMatrix(0.0) * glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
	AssertEqual(
		"wrong position at t=0",
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(pos) / pos.w
	);

	pos = orbit.InverseMatrix(PI * 0.5) * glm::vec4(0.0f, 0.70710676908493f, -0.70710676908493f, 1.0f);
	AssertEqual(
		"wrong position at t=90°",
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(pos) / pos.w
	);

	pos = orbit.InverseMatrix(PI) * glm::vec4(-1.0f, 0.0f, 0.0f, 1.0f);
	AssertEqual(
		"wrong position at t=180°",
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(pos) / pos.w
	);

	pos = orbit.InverseMatrix(PI * 1.5) * glm::vec4(0.0f, -0.70710676908493f, 0.70710676908493f, 1.0f);
	AssertEqual(
		"wrong position at t=270°",
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(pos) / pos.w
	);

	pos = orbit.InverseMatrix(PI * 2.0) * glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
	AssertEqual(
		"wrong position at t=360°",
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(pos) / pos.w
	);
}

void OrbitTest::testInverseLngAsc() {
	Orbit orbit;
	orbit.LongitudeAscending(PI * 0.25); // 45°
	orbit.Inclination(PI * 0.5);

	// inclination rotates counter clockwise around +X, while LAN rotates it
	// around +Y, so at t=0 should be at (sma*sin(45°),0,-sma*cos(45°))
	glm::vec4 pos(orbit.InverseMatrix(0.0) * glm::vec4(0.70710676908493f, 0.0f, -0.70710676908493f, 1.0f));
	AssertEqual(
		"wrong position at t=0",
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(pos) / pos.w
	);

	pos = orbit.InverseMatrix(PI * 0.5) * glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
	AssertEqual(
		"wrong position at t=90°",
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(pos) / pos.w
	);

	pos = orbit.InverseMatrix(PI) * glm::vec4(-0.70710676908493f, 0.0f, 0.70710676908493f, 1.0f);
	AssertEqual(
		"wrong position at t=180°",
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(pos) / pos.w
	);

	pos = orbit.InverseMatrix(PI * 1.5) * glm::vec4(0.0f, -1.0f, 0.0f, 1.0f);
	AssertEqual(
		"wrong position at t=270°",
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(pos) / pos.w
	);

	pos = orbit.InverseMatrix(PI * 2.0) * glm::vec4(0.70710676908493f, 0.0f, -0.70710676908493f, 1.0f);
	AssertEqual(
		"wrong position at t=360°",
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(pos) / pos.w
	);
}

void OrbitTest::testInverseArgPe() {
	Orbit orbit;
	orbit.ArgumentPeriapsis(PI * 0.25); // 45°
	orbit.Inclination(PI * 0.5);

	// inclination rotates counter clockwise around +X, while APe rotates it
	// around +Y in the rotated coordinate system, so at t=0 should be at
	// (sma*sin(45°),0,sma*cos(45°))
	glm::vec4 pos(orbit.InverseMatrix(0.0) * glm::vec4(0.70710676908493f, 0.0f, 0.70710676908493f, 1.0f));
	AssertEqual(
		"wrong position at t=0",
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(pos) / pos.w
	);

	pos = orbit.InverseMatrix(PI * 0.5) * glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
	AssertEqual(
		"wrong position at t=90°",
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(pos) / pos.w
	);

	pos = orbit.InverseMatrix(PI) * glm::vec4(-0.70710676908493f, 0.0f, -0.70710676908493f, 1.0f);
	AssertEqual(
		"wrong position at t=180°",
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(pos) / pos.w
	);

	pos = orbit.InverseMatrix(PI * 1.5) * glm::vec4(0.0f, -1.0f, 0.0f, 1.0f);
	AssertEqual(
		"wrong position at t=270°",
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(pos) / pos.w
	);

	pos = orbit.InverseMatrix(PI * 2.0) * glm::vec4(0.70710676908493f, 0.0f, 0.70710676908493f, 1.0f);
	AssertEqual(
		"wrong position at t=360°",
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(pos) / pos.w
	);
}

void OrbitTest::testInverseMnAn() {
	Orbit orbit;
	orbit.MeanAnomaly(PI * 0.25); // 45°

	// mean anomaly just phase shifts the orbit
	glm::vec4 pos(orbit.InverseMatrix(0.0) * glm::vec4(0.70710676908493f, 0.0f, -0.70710676908493f, 1.0f));
	AssertEqual(
		"wrong position at t=0",
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(pos) / pos.w
	);

	// at 90° position should be (0,0,sma) since the zero inclination
	// reference plane is XZ and rotates counter-clockwise
	pos = orbit.InverseMatrix(PI * 0.5) * glm::vec4(-0.70710676908493f, 0.0f, -0.70710676908493f, 1.0f);
	AssertEqual(
		"wrong position at t=90°",
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(pos) / pos.w
	);

	// at 180° position should be (-sma,0,0)
	pos = orbit.InverseMatrix(PI) * glm::vec4(-0.70710676908493f, 0.0f, 0.70710676908493f, 1.0f);
	AssertEqual(
		"wrong position at t=180°",
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(pos) / pos.w
	);

	// at 270° position should be (0,0,-sma)
	pos = orbit.InverseMatrix(PI * 1.5) * glm::vec4(0.70710676908493f, 0.0f, 0.70710676908493f, 1.0f);
	AssertEqual(
		"wrong position at t=270°",
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(pos) / pos.w
	);

	// at 360° position should be (sma,0,0), the initial position
	pos = orbit.InverseMatrix(PI * 2.0) * glm::vec4(0.70710676908493f, 0.0f, -0.70710676908493f, 1.0f);
	AssertEqual(
		"wrong position at t=360°",
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(pos) / pos.w
	);
}
}
}
}
