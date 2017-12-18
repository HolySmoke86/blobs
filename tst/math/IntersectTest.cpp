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
	glm::dmat4 Ma(1); // identity
	glm::dmat4 Mb(1); // identity
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
		0.014213562373095, depth, delta
	);
	AssertEqual(
		"bad intersection normal (with rotation)",
		glm::dvec3(1, 0, 0), glm::abs(normal) // normal can be in + or - x, therefore abs()
	);

	Mb = glm::translate(glm::dvec3(3, 0, 0)); // 3 to the right
	CPPUNIT_ASSERT_MESSAGE(
		"OBBs intersect (one rotated by 45°)",
		!Intersect(box, Ma, box, Mb, normal, depth)
	);
}

void IntersectTest::testRaySphereIntersection() {
	const double epsilon = std::numeric_limits<double>::epsilon();
	Ray ray{ { 0.0, 0.0, 0.0 }, { 1.0, 0.0, 0.0 } }; // at origin, pointing right
	Sphere sphere{ { 0.0, 0.0, 0.0 }, 1.0 }; // unit sphere at origin

	glm::dvec3 normal(0.0);
	double dist = 0.0;
	CPPUNIT_ASSERT_MESSAGE(
		"ray at origin does not intersect sphere at origin",
		Intersect(ray, sphere, normal, dist)
	);
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
		"distance along ray to unit sphere, both at origin, is not 1",
		1.0, dist, epsilon
	);
	AssertEqual(
		"bad intersection normal",
		glm::dvec3(1.0, 0.0, 0.0), normal
	);

	ray.Origin({ 0.5, 0.0, 0.0 }); // a tad to the right
	CPPUNIT_ASSERT_MESSAGE(
		"ray does not intersect sphere at origin",
		Intersect(ray, sphere, normal, dist)
	);
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
		"distance along ray to unit sphere at origin is not 0.5",
		0.5, dist, epsilon
	);
	AssertEqual(
		"bad intersection normal",
		glm::dvec3(1.0, 0.0, 0.0), normal
	);

	// corner case: ray origin exactly on sphere (should "intersect")
	ray = glm::translate(glm::dvec3(0.5, 0, 0)) * ray;
	CPPUNIT_ASSERT_MESSAGE(
		"ray touching sphere does not intersect it",
		Intersect(ray, sphere, normal, dist)
	);
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
		"distance along ray touching unit sphere is not 0.0",
		0.0, dist, epsilon
	);
	AssertEqual(
		"bad intersection normal",
		glm::dvec3(1.0, 0.0, 0.0), normal
	);

	ray.Origin({ 2.0, 0.0, 0.0 }); // move outside
	CPPUNIT_ASSERT_MESSAGE(
		"ray pointing away from sphere intersects it for some reason",
		!Intersect(ray, sphere, normal, dist)
	);

	ray.Direction({ -1.0, 0.0, 0.0 }); // flip it around
	CPPUNIT_ASSERT_MESSAGE(
		"negative X ray does not intersect sphere",
		Intersect(ray, sphere, normal, dist)
	);
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
		"distance along ray to unit sphere at origin is not 1",
		1.0, dist, epsilon
	);
	AssertEqual(
		"bad intersection normal",
		glm::dvec3(1.0, 0.0, 0.0), normal
	);

	// sphere at 3,0,0; ray at 0,4,0 pointing directly at it
	// should be 5 units apart, minus one for the radius
	ray.Origin({ 0.0, 4.0, 0.0 });
	ray.Direction(glm::normalize(glm::dvec3(3.0, -4.0, 0.0)));
	sphere = glm::translate(glm::dvec3(3.0, 0, 0)) * sphere;
	sphere.origin = { 3.0, 0.0, 0.0 };
	CPPUNIT_ASSERT_MESSAGE(
		"diagonal ray does not intersect sphere",
		Intersect(ray, sphere, normal, dist)
	);
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
		"distance along ray to unit sphere is not 4",
		4.0, dist, epsilon
	);
	AssertEqual(
		"bad intersection normal",
		glm::normalize(glm::dvec3(-3.0, 4.0, 0.0)), normal
	);

	// point the ray straight down, so it misses
	ray.Direction({ 0.0, -1.0, 0.0});
	CPPUNIT_ASSERT_MESSAGE(
		"vertical ray should not intersect sphere",
		!Intersect(ray, sphere, normal, dist)
	);
}

}
}
}
