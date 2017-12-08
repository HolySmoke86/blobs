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
	constexpr double epsilon = 10.0 * std::numeric_limits<double>::epsilon();

	Orbit orbit;
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
		"wrong semi-major axis on default orbit",
		1.0, orbit.SemiMajorAxis(), epsilon
	);
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
		"wrong eccentricity on default orbit",
		0.0, orbit.Eccentricity(), epsilon
	);
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
		"wrong inclination on default orbit",
		0.0, orbit.Inclination(), epsilon
	);
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
		"wrong longitude of ascending node on default orbit",
		0.0, orbit.LongitudeAscending(), epsilon
	);
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
		"wrong argument of periapsis on default orbit",
		0.0, orbit.ArgumentPeriapsis(), epsilon
	);
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
		"wrong mean anomaly on default orbit",
		0.0, orbit.MeanAnomaly(), epsilon
	);

	// reference direction is +X, so at t=0, the body should be
	// at (sma,0,0) relative to its parent
	glm::dvec4 pos(orbit.Matrix(0.0) * glm::dvec4(0.0, 0.0, 0.0, 1.0));
	AssertEqual(
		"wrong position at t=0",
		glm::dvec3(1.0, 0.0, 0.0),
		glm::dvec3(pos) / pos.w,
		epsilon
	);

	// at 90° position should be (0,0,sma) since the zero inclination
	// reference plane is XZ and rotates counter-clockwise
	pos = orbit.Matrix(PI * 0.5) * glm::dvec4(0.0, 0.0, 0.0, 1.0);
	AssertEqual(
		"wrong position at t=90°",
		glm::dvec3(0.0, 0.0, -1.0),
		glm::dvec3(pos) / pos.w,
		epsilon
	);

	// at 180° position should be (-sma,0,0)
	pos = orbit.Matrix(PI) * glm::dvec4(0.0, 0.0, 0.0, 1.0);
	AssertEqual(
		"wrong position at t=180°",
		glm::dvec3(-1.0, 0.0, 0.0),
		glm::dvec3(pos) / pos.w,
		epsilon
	);

	// at 270° position should be (0,0,-sma)
	pos = orbit.Matrix(PI * 1.5) * glm::dvec4(0.0, 0.0, 0.0, 1.0);
	AssertEqual(
		"wrong position at t=270°",
		glm::dvec3(0.0, 0.0, 1.0),
		glm::dvec3(pos) / pos.w,
		epsilon
	);

	// at 360° position should be (sma,0,0), the initial position
	pos = orbit.Matrix(PI * 2.0) * glm::dvec4(0.0, 0.0, 0.0, 1.0);
	AssertEqual(
		"wrong position at t=360°",
		glm::dvec3(1.0, 0.0, 0.0),
		glm::dvec3(pos) / pos.w,
		epsilon
	);
}

void OrbitTest::testSMA() {
	constexpr double epsilon = 10.0 * std::numeric_limits<double>::epsilon();

	Orbit orbit;
	orbit.SemiMajorAxis(2.0);
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
		"wrong semi-major axis on orbit",
		2.0, orbit.SemiMajorAxis(), epsilon
	);

	// reference direction is +X, so at t=0, the body should be
	// at (sma,0,0) relative to its parent
	glm::dvec4 pos(orbit.Matrix(0.0) * glm::dvec4(0.0, 0.0, 0.0, 1.0));
	AssertEqual(
		"wrong position at t=0",
		glm::dvec3(2.0, 0.0, 0.0),
		glm::dvec3(pos) / pos.w,
		epsilon
	);

	// at 90° position should be (0,0,sma) since the zero inclination
	// reference plane is XZ and rotates counter-clockwise
	pos = orbit.Matrix(PI * 0.5) * glm::dvec4(0.0, 0.0, 0.0, 1.0);
	AssertEqual(
		"wrong position at t=90°",
		glm::dvec3(0.0, 0.0, -2.0),
		glm::dvec3(pos) / pos.w,
		epsilon
	);

	// at 180° position should be (-sma,0,0)
	pos = orbit.Matrix(PI) * glm::dvec4(0.0, 0.0, 0.0, 1.0);
	AssertEqual(
		"wrong position at t=180°",
		glm::dvec3(-2.0, 0.0, 0.0),
		glm::dvec3(pos) / pos.w,
		epsilon
	);

	// at 270° position should be (0,0,-sma)
	pos = orbit.Matrix(PI * 1.5) * glm::dvec4(0.0, 0.0, 0.0, 1.0);
	AssertEqual(
		"wrong position at t=270°",
		glm::dvec3(0.0, 0.0, 2.0),
		glm::dvec3(pos) / pos.w,
		epsilon
	);

	// at 360° position should be (sma,0,0), the initial position
	pos = orbit.Matrix(PI * 2.0) * glm::dvec4(0.0, 0.0, 0.0, 1.0);
	AssertEqual(
		"wrong position at t=360°",
		glm::dvec3(2.0, 0.0, 0.0),
		glm::dvec3(pos) / pos.w,
		epsilon
	);
}

void OrbitTest::testEcc() {
	constexpr double epsilon = 10.0 * std::numeric_limits<double>::epsilon();

	Orbit orbit;
	orbit.Eccentricity(0.5);
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
		"wrong eccentricity on orbit",
		0.5, orbit.Eccentricity(), epsilon
	);

	glm::dvec4 pos(orbit.Matrix(0.0) * glm::dvec4(0.0, 0.0, 0.0, 1.0));
	AssertEqual(
		"wrong position at t=0",
		glm::dvec3(0.5, 0.0, 0.0),
		glm::dvec3(pos) / pos.w,
		epsilon
	);

	pos = orbit.Matrix(PI * 0.5) * glm::dvec4(0.0, 0.0, 0.0, 1.0);
	AssertEqual(
		"wrong position at t=90°",
		glm::dvec3(-0.93513085903671, 0.0, -0.779740887497559),
		glm::dvec3(pos) / pos.w,
		epsilon
	);

	pos = orbit.Matrix(PI) * glm::dvec4(0.0, 0.0, 0.0, 1.0);
	AssertEqual(
		"wrong position at t=180°",
		glm::dvec3(-1.5, 0.0, 0.0),
		glm::dvec3(pos) / pos.w,
		epsilon
	);

	pos = orbit.Matrix(PI * 1.5) * glm::dvec4(0.0, 0.0, 0.0, 1.0);
	AssertEqual(
		"wrong position at t=270°",
		glm::dvec3(-0.93513085903671, 0.0, 0.779740887497559),
		glm::dvec3(pos) / pos.w,
		epsilon
	);

	pos = orbit.Matrix(PI * 2.0) * glm::dvec4(0.0, 0.0, 0.0, 1.0);
	AssertEqual(
		"wrong position at t=360°",
		glm::dvec3(0.5, 0.0, 0.0),
		glm::dvec3(pos) / pos.w,
		epsilon
	);
}

void OrbitTest::testInc() {
	constexpr double epsilon = 10.0 * std::numeric_limits<double>::epsilon();

	Orbit orbit;
	orbit.Inclination(PI * 0.25); // 45°
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
		"wrong inclination on orbit",
		PI * 0.25, orbit.Inclination(), epsilon
	);

	// inclination rotates counter clockwise around +X, so at t=0 should be
	// at (sma,0,0) relative to its parent
	glm::dvec4 pos(orbit.Matrix(0.0) * glm::dvec4(0.0, 0.0, 0.0, 1.0));
	AssertEqual(
		"wrong position at t=0",
		glm::dvec3(1.0, 0.0, 0.0),
		glm::dvec3(pos) / pos.w,
		epsilon
	);

	pos = orbit.Matrix(PI * 0.5) * glm::dvec4(0.0, 0.0, 0.0, 1.0);
	AssertEqual(
		"wrong position at t=90°",
		glm::dvec3(0.0, 0.707106781186548, -0.707106781186548),
		glm::dvec3(pos) / pos.w,
		epsilon
	);

	pos = orbit.Matrix(PI) * glm::dvec4(0.0, 0.0, 0.0, 1.0);
	AssertEqual(
		"wrong position at t=180°",
		glm::dvec3(-1.0, 0.0, 0.0),
		glm::dvec3(pos) / pos.w,
		epsilon
	);

	pos = orbit.Matrix(PI * 1.5) * glm::dvec4(0.0, 0.0, 0.0, 1.0);
	AssertEqual(
		"wrong position at t=270°",
		glm::dvec3(0.0, -0.707106781186548, 0.707106781186548),
		glm::dvec3(pos) / pos.w,
		epsilon
	);

	pos = orbit.Matrix(PI * 2.0) * glm::dvec4(0.0, 0.0, 0.0, 1.0);
	AssertEqual(
		"wrong position at t=360°",
		glm::dvec3(1.0, 0.0, 0.0),
		glm::dvec3(pos) / pos.w,
		epsilon
	);
}

void OrbitTest::testLngAsc() {
	constexpr double epsilon = 10.0 * std::numeric_limits<double>::epsilon();

	Orbit orbit;
	orbit.LongitudeAscending(PI * 0.25); // 45°
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
		"wrong longitude of ascending node on orbit",
		PI * 0.25, orbit.LongitudeAscending(), epsilon
	);
	// using an inclination of 90° as well to make the rotation more apparent
	orbit.Inclination(PI * 0.5);

	// inclination rotates counter clockwise around +X, while LAN rotates it
	// around +Y, so at t=0 should be at (sma*sin(45°),0,-sma*cos(45°))
	glm::dvec4 pos(orbit.Matrix(0.0) * glm::dvec4(0.0, 0.0, 0.0, 1.0));
	AssertEqual(
		"wrong position at t=0",
		glm::dvec3(0.707106781186548, 0.0, -0.707106781186548),
		glm::dvec3(pos) / pos.w,
		epsilon
	);

	pos = orbit.Matrix(PI * 0.5) * glm::dvec4(0.0, 0.0, 0.0, 1.0);
	AssertEqual(
		"wrong position at t=90°",
		glm::dvec3(0.0, 1.0, 0.0),
		glm::dvec3(pos) / pos.w,
		epsilon
	);

	pos = orbit.Matrix(PI) * glm::dvec4(0.0, 0.0, 0.0, 1.0);
	AssertEqual(
		"wrong position at t=180°",
		glm::dvec3(-0.707106781186548, 0.0, 0.707106781186548),
		glm::dvec3(pos) / pos.w,
		epsilon
	);

	pos = orbit.Matrix(PI * 1.5) * glm::dvec4(0.0, 0.0, 0.0, 1.0);
	AssertEqual(
		"wrong position at t=270°",
		glm::dvec3(0.0, -1.0, 0.0),
		glm::dvec3(pos) / pos.w,
		epsilon
	);

	pos = orbit.Matrix(PI * 2.0) * glm::dvec4(0.0, 0.0, 0.0, 1.0);
	AssertEqual(
		"wrong position at t=360°",
		glm::dvec3(0.707106781186548, 0.0, -0.707106781186548),
		glm::dvec3(pos) / pos.w,
		epsilon
	);
}

void OrbitTest::testArgPe() {
	constexpr double epsilon = 10.0 * std::numeric_limits<double>::epsilon();

	Orbit orbit;
	orbit.ArgumentPeriapsis(PI * 0.25); // 45°
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
		"wrong argument of periapsis node on orbit",
		PI * 0.25, orbit.ArgumentPeriapsis(), epsilon
	);
	// using an inclination of 90° as well to make the rotation more apparent
	orbit.Inclination(PI * 0.5);

	// inclination rotates counter clockwise around +X, while APe rotates it
	// around +Y in the rotated coordinate system, so at t=0 should be at
	// (sma*sin(45°),0,sma*cos(45°))
	glm::dvec4 pos(orbit.Matrix(0.0) * glm::dvec4(0.0, 0.0, 0.0, 1.0));
	AssertEqual(
		"wrong position at t=0",
		glm::dvec3(0.707106781186548, 0.0, 0.707106781186548),
		glm::dvec3(pos) / pos.w,
		epsilon
	);

	pos = orbit.Matrix(PI * 0.5) * glm::dvec4(0.0, 0.0, 0.0, 1.0);
	AssertEqual(
		"wrong position at t=90°",
		glm::dvec3(0.0, 1.0, 0.0),
		glm::dvec3(pos) / pos.w,
		epsilon
	);

	pos = orbit.Matrix(PI) * glm::dvec4(0.0, 0.0, 0.0, 1.0);
	AssertEqual(
		"wrong position at t=180°",
		glm::dvec3(-0.707106781186548, 0.0, -0.707106781186548),
		glm::dvec3(pos) / pos.w,
		epsilon
	);

	pos = orbit.Matrix(PI * 1.5) * glm::dvec4(0.0, 0.0, 0.0, 1.0);
	AssertEqual(
		"wrong position at t=270°",
		glm::dvec3(0.0, -1.0, 0.0),
		glm::dvec3(pos) / pos.w,
		epsilon
	);

	pos = orbit.Matrix(PI * 2.0) * glm::dvec4(0.0, 0.0, 0.0, 1.0);
	AssertEqual(
		"wrong position at t=360°",
		glm::dvec3(0.707106781186548, 0.0, 0.707106781186548),
		glm::dvec3(pos) / pos.w,
		epsilon
	);
}

void OrbitTest::testMnAn() {
	constexpr double epsilon = 10.0 * std::numeric_limits<double>::epsilon();

	Orbit orbit;
	orbit.MeanAnomaly(PI * 0.25); // 45°
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
		"wrong mean anomaly on default orbit",
		PI * 0.25, orbit.MeanAnomaly(), epsilon
	);

	// mean anomaly just phase shifts the orbit
	glm::dvec4 pos(orbit.Matrix(0.0) * glm::dvec4(0.0, 0.0, 0.0, 1.0));
	AssertEqual(
		"wrong position at t=0",
		glm::dvec3(0.707106781186548, 0.0, -0.707106781186548),
		glm::dvec3(pos) / pos.w,
		epsilon
	);

	// at 90° position should be (0,0,sma) since the zero inclination
	// reference plane is XZ and rotates counter-clockwise
	pos = orbit.Matrix(PI * 0.5) * glm::dvec4(0.0, 0.0, 0.0, 1.0);
	AssertEqual(
		"wrong position at t=90°",
		glm::dvec3(-0.707106781186548, 0.0, -0.707106781186548),
		glm::dvec3(pos) / pos.w,
		epsilon
	);

	// at 180° position should be (-sma,0,0)
	pos = orbit.Matrix(PI) * glm::dvec4(0.0, 0.0, 0.0, 1.0);
	AssertEqual(
		"wrong position at t=180°",
		glm::dvec3(-0.707106781186548, 0.0, 0.707106781186548),
		glm::dvec3(pos) / pos.w,
		epsilon
	);

	// at 270° position should be (0,0,-sma)
	pos = orbit.Matrix(PI * 1.5) * glm::dvec4(0.0, 0.0, 0.0, 1.0);
	AssertEqual(
		"wrong position at t=270°",
		glm::dvec3(0.707106781186548, 0.0, 0.707106781186548),
		glm::dvec3(pos) / pos.w,
		epsilon
	);

	// at 360° position should be (sma,0,0), the initial position
	pos = orbit.Matrix(PI * 2.0) * glm::dvec4(0.0, 0.0, 0.0, 1.0);
	AssertEqual(
		"wrong position at t=360°",
		glm::dvec3(0.707106781186548, 0.0, -0.707106781186548),
		glm::dvec3(pos) / pos.w,
		epsilon
	);
}

void OrbitTest::testInverseDefault() {
	constexpr double epsilon = 10.0 * std::numeric_limits<double>::epsilon();

	Orbit orbit;

	// inverse matrix should project expected orbit position back to the origin
	glm::dvec4 pos(orbit.InverseMatrix(0.0) * glm::dvec4(1.0, 0.0, 0.0, 1.0));
	AssertEqual(
		"wrong position at t=0",
		glm::dvec3(0.0, 0.0, 0.0),
		glm::dvec3(pos) / pos.w,
		epsilon
	);

	// at 90° position should be (0,0,sma) since the zero inclination
	// reference plane is XZ and rotates counter-clockwise
	pos = orbit.InverseMatrix(PI * 0.5) * glm::dvec4(0.0, 0.0, -1.0, 1.0);
	AssertEqual(
		"wrong position at t=90°",
		glm::dvec3(0.0, 0.0, 0.0),
		glm::dvec3(pos) / pos.w,
		epsilon
	);

	// at 180° position should be (-sma,0,0)
	pos = orbit.InverseMatrix(PI) * glm::dvec4(-1.0, 0.0, 0.0, 1.0);
	AssertEqual(
		"wrong position at t=180°",
		glm::dvec3(0.0, 0.0, 0.0),
		glm::dvec3(pos) / pos.w,
		epsilon
	);

	// at 270° position should be (0,0,-sma)
	pos = orbit.InverseMatrix(PI * 1.5) * glm::dvec4(0.0, 0.0, 1.0, 1.0);
	AssertEqual(
		"wrong position at t=270°",
		glm::dvec3(0.0, 0.0, 0.0),
		glm::dvec3(pos) / pos.w,
		epsilon
	);

	// at 360° position should be (sma,0,0), the initial position
	pos = orbit.InverseMatrix(PI * 2.0) * glm::dvec4(1.0, 0.0, 0.0, 1.0);
	AssertEqual(
		"wrong position at t=360°",
		glm::dvec3(0.0, 0.0, 0.0),
		glm::dvec3(pos) / pos.w,
		epsilon
	);
}

void OrbitTest::testInverseSMA() {
	constexpr double epsilon = 10.0 * std::numeric_limits<double>::epsilon();

	Orbit orbit;
	orbit.SemiMajorAxis(2.0);

	// reference direction is +X, so at t=0, the body should be
	// at (sma,0,0) relative to its parent
	glm::dvec4 pos(orbit.InverseMatrix(0.0) * glm::dvec4(2.0, 0.0, 0.0, 1.0));
	AssertEqual(
		"wrong position at t=0",
		glm::dvec3(0.0, 0.0, 0.0),
		glm::dvec3(pos) / pos.w,
		epsilon
	);

	// at 90° position should be (0,0,sma) since the zero inclination
	// reference plane is XZ and rotates counter-clockwise
	pos = orbit.InverseMatrix(PI * 0.5) * glm::dvec4(0.0, 0.0, -2.0, 1.0);
	AssertEqual(
		"wrong position at t=90°",
		glm::dvec3(0.0, 0.0, 0.0),
		glm::dvec3(pos) / pos.w,
		epsilon
	);

	// at 180° position should be (-sma,0,0)
	pos = orbit.InverseMatrix(PI) * glm::dvec4(-2.0, 0.0, 0.0, 1.0);
	AssertEqual(
		"wrong position at t=180°",
		glm::dvec3(0.0, 0.0, 0.0),
		glm::dvec3(pos) / pos.w,
		epsilon
	);

	// at 270° position should be (0,0,-sma)
	pos = orbit.InverseMatrix(PI * 1.5) * glm::dvec4(0.0, 0.0, 2.0, 1.0);
	AssertEqual(
		"wrong position at t=270°",
		glm::dvec3(0.0, 0.0, 0.0),
		glm::dvec3(pos) / pos.w,
		epsilon
	);

	// at 360° position should be (sma,0,0), the initial position
	pos = orbit.InverseMatrix(PI * 2.0) * glm::dvec4(2.0, 0.0, 0.0, 1.0);
	AssertEqual(
		"wrong position at t=360°",
		glm::dvec3(0.0, 0.0, 0.0),
		glm::dvec3(pos) / pos.w,
		epsilon
	);
}

void OrbitTest::testInverseEcc() {
	constexpr double epsilon = 10.0 * std::numeric_limits<double>::epsilon();

	Orbit orbit;
	orbit.Eccentricity(0.5);

	glm::dvec4 pos(orbit.InverseMatrix(0.0) * glm::dvec4(0.5, 0.0, 0.0, 1.0));
	AssertEqual(
		"wrong position at t=0",
		glm::dvec3(0.0, 0.0, 0.0),
		glm::dvec3(pos) / pos.w,
		epsilon
	);

	pos = orbit.InverseMatrix(PI * 0.5) * glm::dvec4(-0.93513085903671, 0.0, -0.779740887497559, 1.0);
	AssertEqual(
		"wrong position at t=90°",
		glm::dvec3(0.0, 0.0, 0.0),
		glm::dvec3(pos) / pos.w,
		epsilon
	);

	pos = orbit.InverseMatrix(PI) * glm::dvec4(-1.5, 0.0, 0.0, 1.0);
	AssertEqual(
		"wrong position at t=180°",
		glm::dvec3(0.0, 0.0, 0.0),
		glm::dvec3(pos) / pos.w,
		epsilon
	);

	pos = orbit.InverseMatrix(PI * 1.5) * glm::dvec4(-0.93513085903671, 0.0, 0.779740887497559, 1.0);
	AssertEqual(
		"wrong position at t=270°",
		glm::dvec3(0.0, 0.0, 0.0),
		glm::dvec3(pos) / pos.w,
		epsilon
	);

	pos = orbit.InverseMatrix(PI * 2.0) * glm::dvec4(0.5, 0.0, 0.0, 1.0);
	AssertEqual(
		"wrong position at t=360°",
		glm::dvec3(0.0, 0.0, 0.0),
		glm::dvec3(pos) / pos.w,
		epsilon
	);
}

void OrbitTest::testInverseInc() {
	constexpr double epsilon = 10.0 * std::numeric_limits<double>::epsilon();

	Orbit orbit;
	orbit.Inclination(PI * 0.25); // 45°

	// inclination rotates counter clockwise around +X, so at t=0 should be
	// at (sma,0,0) relative to its parent
	glm::dvec4 pos(orbit.InverseMatrix(0.0) * glm::dvec4(1.0, 0.0, 0.0, 1.0));
	AssertEqual(
		"wrong position at t=0",
		glm::dvec3(0.0, 0.0, 0.0),
		glm::dvec3(pos) / pos.w,
		epsilon
	);

	pos = orbit.InverseMatrix(PI * 0.5) * glm::dvec4(0.0, 0.707106781186548, -0.707106781186548, 1.0);
	AssertEqual(
		"wrong position at t=90°",
		glm::dvec3(0.0, 0.0, 0.0),
		glm::dvec3(pos) / pos.w,
		epsilon
	);

	pos = orbit.InverseMatrix(PI) * glm::dvec4(-1.0, 0.0, 0.0, 1.0);
	AssertEqual(
		"wrong position at t=180°",
		glm::dvec3(0.0, 0.0, 0.0),
		glm::dvec3(pos) / pos.w,
		epsilon
	);

	pos = orbit.InverseMatrix(PI * 1.5) * glm::dvec4(0.0, -0.707106781186548, 0.707106781186548, 1.0);
	AssertEqual(
		"wrong position at t=270°",
		glm::dvec3(0.0, 0.0, 0.0),
		glm::dvec3(pos) / pos.w,
		epsilon
	);

	pos = orbit.InverseMatrix(PI * 2.0) * glm::dvec4(1.0, 0.0, 0.0, 1.0);
	AssertEqual(
		"wrong position at t=360°",
		glm::dvec3(0.0, 0.0, 0.0),
		glm::dvec3(pos) / pos.w,
		epsilon
	);
}

void OrbitTest::testInverseLngAsc() {
	constexpr double epsilon = 10.0 * std::numeric_limits<double>::epsilon();

	Orbit orbit;
	orbit.LongitudeAscending(PI * 0.25); // 45°
	orbit.Inclination(PI * 0.5);

	// inclination rotates counter clockwise around +X, while LAN rotates it
	// around +Y, so at t=0 should be at (sma*sin(45°),0,-sma*cos(45°))
	glm::dvec4 pos(orbit.InverseMatrix(0.0) * glm::dvec4(0.707106781186548, 0.0, -0.707106781186548, 1.0));
	AssertEqual(
		"wrong position at t=0",
		glm::dvec3(0.0, 0.0, 0.0),
		glm::dvec3(pos) / pos.w,
		epsilon
	);

	pos = orbit.InverseMatrix(PI * 0.5) * glm::dvec4(0.0, 1.0, 0.0, 1.0);
	AssertEqual(
		"wrong position at t=90°",
		glm::dvec3(0.0, 0.0, 0.0),
		glm::dvec3(pos) / pos.w,
		epsilon
	);

	pos = orbit.InverseMatrix(PI) * glm::dvec4(-0.707106781186548, 0.0, 0.707106781186548, 1.0);
	AssertEqual(
		"wrong position at t=180°",
		glm::dvec3(0.0, 0.0, 0.0),
		glm::dvec3(pos) / pos.w,
		epsilon
	);

	pos = orbit.InverseMatrix(PI * 1.5) * glm::dvec4(0.0, -1.0, 0.0, 1.0);
	AssertEqual(
		"wrong position at t=270°",
		glm::dvec3(0.0, 0.0, 0.0),
		glm::dvec3(pos) / pos.w,
		epsilon
	);

	pos = orbit.InverseMatrix(PI * 2.0) * glm::dvec4(0.707106781186548, 0.0, -0.707106781186548, 1.0);
	AssertEqual(
		"wrong position at t=360°",
		glm::dvec3(0.0, 0.0, 0.0),
		glm::dvec3(pos) / pos.w,
		epsilon
	);
}

void OrbitTest::testInverseArgPe() {
	constexpr double epsilon = 10.0 * std::numeric_limits<double>::epsilon();

	Orbit orbit;
	orbit.ArgumentPeriapsis(PI * 0.25); // 45°
	orbit.Inclination(PI * 0.5);

	// inclination rotates counter clockwise around +X, while APe rotates it
	// around +Y in the rotated coordinate system, so at t=0 should be at
	// (sma*sin(45°),0,sma*cos(45°))
	glm::dvec4 pos(orbit.InverseMatrix(0.0) * glm::dvec4(0.707106781186548, 0.0, 0.707106781186548, 1.0));
	AssertEqual(
		"wrong position at t=0",
		glm::dvec3(0.0, 0.0, 0.0),
		glm::dvec3(pos) / pos.w,
		epsilon
	);

	pos = orbit.InverseMatrix(PI * 0.5) * glm::dvec4(0.0, 1.0, 0.0, 1.0);
	AssertEqual(
		"wrong position at t=90°",
		glm::dvec3(0.0, 0.0, 0.0),
		glm::dvec3(pos) / pos.w,
		epsilon
	);

	pos = orbit.InverseMatrix(PI) * glm::dvec4(-0.707106781186548, 0.0, -0.707106781186548, 1.0);
	AssertEqual(
		"wrong position at t=180°",
		glm::dvec3(0.0, 0.0, 0.0),
		glm::dvec3(pos) / pos.w,
		epsilon
	);

	pos = orbit.InverseMatrix(PI * 1.5) * glm::dvec4(0.0, -1.0, 0.0, 1.0);
	AssertEqual(
		"wrong position at t=270°",
		glm::dvec3(0.0, 0.0, 0.0),
		glm::dvec3(pos) / pos.w,
		epsilon
	);

	pos = orbit.InverseMatrix(PI * 2.0) * glm::dvec4(0.707106781186548, 0.0, 0.707106781186548, 1.0);
	AssertEqual(
		"wrong position at t=360°",
		glm::dvec3(0.0, 0.0, 0.0),
		glm::dvec3(pos) / pos.w,
		epsilon
	);
}

void OrbitTest::testInverseMnAn() {
	constexpr double epsilon = 10.0 * std::numeric_limits<double>::epsilon();

	Orbit orbit;
	orbit.MeanAnomaly(PI * 0.25); // 45°

	// mean anomaly just phase shifts the orbit
	glm::dvec4 pos(orbit.InverseMatrix(0.0) * glm::dvec4(0.707106781186548, 0.0, -0.707106781186548, 1.0));
	AssertEqual(
		"wrong position at t=0",
		glm::dvec3(0.0, 0.0, 0.0),
		glm::dvec3(pos) / pos.w,
		epsilon
	);

	// at 90° position should be (0,0,sma) since the zero inclination
	// reference plane is XZ and rotates counter-clockwise
	pos = orbit.InverseMatrix(PI * 0.5) * glm::dvec4(-0.707106781186548, 0.0, -0.707106781186548, 1.0);
	AssertEqual(
		"wrong position at t=90°",
		glm::dvec3(0.0, 0.0, 0.0),
		glm::dvec3(pos) / pos.w,
		epsilon
	);

	// at 180° position should be (-sma,0,0)
	pos = orbit.InverseMatrix(PI) * glm::dvec4(-0.707106781186548, 0.0, 0.707106781186548, 1.0);
	AssertEqual(
		"wrong position at t=180°",
		glm::dvec3(0.0, 0.0, 0.0),
		glm::dvec3(pos) / pos.w,
		epsilon
	);

	// at 270° position should be (0,0,-sma)
	pos = orbit.InverseMatrix(PI * 1.5) * glm::dvec4(0.707106781186548, 0.0, 0.707106781186548, 1.0);
	AssertEqual(
		"wrong position at t=270°",
		glm::dvec3(0.0, 0.0, 0.0),
		glm::dvec3(pos) / pos.w,
		epsilon
	);

	// at 360° position should be (sma,0,0), the initial position
	pos = orbit.InverseMatrix(PI * 2.0) * glm::dvec4(0.707106781186548, 0.0, -0.707106781186548, 1.0);
	AssertEqual(
		"wrong position at t=360°",
		glm::dvec3(0.0, 0.0, 0.0),
		glm::dvec3(pos) / pos.w,
		epsilon
	);
}
}
}
}
