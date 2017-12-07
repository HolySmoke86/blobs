#include "IntersectTest.hpp"

#include "../assert.hpp"

#include "math/const.hpp"
#include "math/geometry.hpp"

#include <limits>
#include <glm/gtx/transform.hpp>

CPPUNIT_TEST_SUITE_REGISTRATION(blobs::math::test::IntersectTest);

using blobs::test::AssertEqual;


namespace blobs {
namespace math {
namespace test {

void IntersectTest::setUp() {
}

void IntersectTest::tearDown() {
}


void IntersectTest::testRayBoxIntersection() {
	Ray ray{ { 0, 0, 0 }, { 1, 0, 0 } }; // at origin, pointing right
	AABB box{ { -1, -1, -1 }, { 1, 1, 1 } }; // 2x2x2 cube centered around origin
	glm::dmat4 M(1); // no transformation

	const double delta = 1.0e-15;

	double distance = 0;
	glm::dvec3 normal(0);

	CPPUNIT_ASSERT_MESSAGE(
		"ray at origin not intersecting box at origin",
		Intersect(ray, box, M, normal, distance)
	);
	// normal undefined, so can't test

	// move ray outside the box, but have it still point at it
	// should be 4 units to the left now
	ray.Origin({ -5, 0, 0 });
	CPPUNIT_ASSERT_MESSAGE(
		"ray pointing at box to the right doesn't intersect",
		Intersect(ray, box, M, normal, distance)
	);
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
		"intersection distance way off",
		4.0, distance, delta
	);
	AssertEqual(
		"wrong surface normal at intersection point",
		glm::dvec3(-1, 0, 0), normal
	);

	// move ray to the other side, so it's pointing away now
	ray.Origin({ 5, 0, 0 });
	CPPUNIT_ASSERT_MESSAGE(
		"ray pointing away from box to the left still intersects",
		!Intersect(ray, box, M, normal, distance)
	);

	// turn ray around
	ray.Direction({ -1, 0, 0 });
	CPPUNIT_ASSERT_MESSAGE(
		"ray pointing at box to the left does not intersect",
		Intersect(ray, box, M, normal, distance)
	);
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
		"intersection distance way off",
		4.0, distance, delta
	);
	AssertEqual(
		"wrong surface normal at intersection point",
		glm::dvec3(1, 0, 0), normal
	);

	// ray below
	ray.Origin({ 0, -5, 0 });
	ray.Direction({ 0, 1, 0 });
	CPPUNIT_ASSERT_MESSAGE(
		"ray pointing at box above does not intersect",
		Intersect(ray, box, M, normal, distance)
	);
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
		"intersection distance way off",
		4.0, distance, delta
	);
	AssertEqual(
		"wrong surface normal at intersection point",
		glm::dvec3(0, -1, 0), normal
	);

	// turn ray around
	ray.Direction({ 0, -1, 0 });
	CPPUNIT_ASSERT_MESSAGE(
		"ray pointing away from box above still intersects",
		!Intersect(ray, box, M, normal, distance)
	);

	// move ray above
	ray.Origin({ 0, 5, 0 });
	CPPUNIT_ASSERT_MESSAGE(
		"ray pointing at box below does not intersect",
		Intersect(ray, box, M, normal, distance)
	);
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
		"intersection distance way off",
		4.0, distance, delta
	);
	AssertEqual(
		"wrong surface normal at intersection point",
		glm::dvec3(0, 1, 0), normal
	);

	// ray behind
	ray.Origin({ 0, 0, -5 });
	ray.Direction({ 0, 0, 1 });
	CPPUNIT_ASSERT_MESSAGE(
		"ray pointing at box in front does not intersect",
		Intersect(ray, box, M, normal, distance)
	);
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
		"intersection distance way off",
		4.0, distance, delta
	);
	AssertEqual(
		"wrong surface normal at intersection point",
		glm::dvec3(0, 0, -1), normal
	);

	// turn ray around
	ray.Direction({ 0, 0, -1 });
	CPPUNIT_ASSERT_MESSAGE(
		"ray pointing away from box in front still intersects",
		!Intersect(ray, box, M, normal, distance)
	);

	// move ray in front
	ray.Origin({ 0, 0, 5 });
	CPPUNIT_ASSERT_MESSAGE(
		"ray pointing at box behind does not intersect",
		Intersect(ray, box, M, normal, distance)
	);
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
		"intersection distance way off",
		4.0, distance, delta
	);
	AssertEqual(
		"wrong surface normal at intersection point",
		glm::dvec3(0, 0, 1), normal
	);

	// 45 deg down from 4 units away, so should be about 4 * sqrt(2)
	ray.Origin({ -5, 4.5, 0 });
	ray.Direction({ 0.70710678118654752440, -0.70710678118654752440, 0 });
	CPPUNIT_ASSERT_MESSAGE(
		"ray pointing at box doesn't intersect",
		Intersect(ray, box, M, normal, distance)
	);
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
		"intersection distance way off",
		5.65685424949238019520, distance, delta
	);
	AssertEqual(
		"wrong surface normal at intersection point",
		glm::dvec3(-1, 0, 0), normal
	);
}

void IntersectTest::testBoxBoxIntersection() {
	const double delta = std::numeric_limits<double>::epsilon();
	double depth = 0;
	glm::dvec3 normal(0);

	AABB box{ { -1, -1, -1 }, { 1, 1, 1 } }; // 2x2x2 cube centered around origin
	glm::mat4 Ma(1); // identity
	glm::mat4 Mb(1); // identity
	// they're identical, so should probably intersect ^^

	CPPUNIT_ASSERT_MESSAGE(
		"identical OBBs don't intersect",
		Intersect(box, Ma, box, Mb, normal, depth)
	);
	// depth is two, but normal can be any
	// (will probably be the first axis of box a, but any is valid)
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
		"penetration depth of coincidental 2x2x2 boxes is not 2",
		2.0, depth, delta
	);

	Ma = glm::translate(glm::dvec3(-2, 0, 0)); // 2 to the left
	Mb = glm::translate(glm::dvec3(2, 0, 0)); // 2 to the right
	CPPUNIT_ASSERT_MESSAGE(
		"distant OBBs intersect (2 apart, no rotation)",
		!Intersect(box, Ma, box, Mb, normal, depth)
	);
	// depth and normal undefined for non-intersecting objects

	Ma = glm::rotate(PI * 0.25, glm::dvec3(0, 0, 1)); // rotated 45° around Z
	Mb = glm::translate(glm::dvec3(2.4, 0, 0)); // 2.4 to the right
	// they should barely touch. intersect by about sqrt(2) - 1.4 if my head works
	CPPUNIT_ASSERT_MESSAGE(
		"OBBs don't intersect (one rotated by 45°)",
		Intersect(box, Ma, box, Mb, normal, depth)
	);
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
		"bad penetration depth (with rotation)",
		0.0142134428024292, depth, delta
	);
	AssertEqual(
		"bad intersection normal (with rotation)",
		glm::dvec3(1, 0, 0), abs(normal) // normal can be in + or - x, therefore abs()
	);

	Mb = glm::translate(glm::dvec3(3, 0, 0)); // 3 to the right
	CPPUNIT_ASSERT_MESSAGE(
		"OBBs intersect (one rotated by 45°)",
		!Intersect(box, Ma, box, Mb, normal, depth)
	);
}

}
}
}
