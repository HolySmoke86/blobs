#include "ColorTest.hpp"

#include "../assert.hpp"

#include "graphics/color.hpp"

#include <limits>

CPPUNIT_TEST_SUITE_REGISTRATION(blobs::graphics::test::ColorTest);

using blobs::test::AssertEqual;


namespace blobs {
namespace graphics {
namespace test {

void ColorTest::setUp() {

}

void ColorTest::tearDown() {

}

void ColorTest::testConversion() {
	const double epsilon = 1.0e-9;
	const glm::dvec3 rgb_black(0.0);
	const glm::dvec3 rgb_white(1.0);
	const glm::dvec3 rgb_red(1.0, 0.0, 0.0);
	const glm::dvec3 rgb_green(0.0, 1.0, 0.0);
	const glm::dvec3 rgb_blue(0.0, 0.0, 1.0);

	glm::dvec3 hsl_result(rgb2hsl(rgb_black));
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
		"wrong saturation for black",
		0.0, hsl_result.y, epsilon
	);
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
		"wrong lightness for black",
		0.0, hsl_result.z, epsilon
	);
	glm::dvec3 rgb_result(hsl2rgb(hsl_result));
	AssertEqual(
		"bad HSL to RGB conversion for black",
		rgb_black, rgb_result, epsilon
	);

	hsl_result = rgb2hsl(rgb_white);
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
		"wrong saturation for white",
		0.0, hsl_result.y, epsilon
	);
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
		"wrong lightness for white",
		1.0, hsl_result.z, epsilon
	);
	rgb_result = hsl2rgb(hsl_result);
	AssertEqual(
		"bad HSL to RGB conversion for white",
		rgb_white, rgb_result, epsilon
	);

	hsl_result = rgb2hsl(rgb_red);
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
		"wrong hue for red",
		0.0, hsl_result.x, epsilon
	);
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
		"wrong saturation for red",
		1.0, hsl_result.y, epsilon
	);
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
		"wrong lightness for red",
		1.0, hsl_result.z, epsilon
	);
	rgb_result = hsl2rgb(hsl_result);
	AssertEqual(
		"bad HSL to RGB conversion for red",
		rgb_red, rgb_result, epsilon
	);

	hsl_result = rgb2hsl(rgb_green);
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
		"wrong hue for green",
		(1.0 / 3.0), hsl_result.x, epsilon
	);
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
		"wrong saturation for green",
		1.0, hsl_result.y, epsilon
	);
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
		"wrong lightness for green",
		1.0, hsl_result.z, epsilon
	);
	rgb_result = hsl2rgb(hsl_result);
	AssertEqual(
		"bad HSL to RGB conversion for green",
		rgb_green, rgb_result, epsilon
	);

	hsl_result = rgb2hsl(rgb_blue);
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
		"wrong hue for blue",
		(2.0 / 3.0), hsl_result.x, epsilon
	);
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
		"wrong saturation for blue",
		1.0, hsl_result.y, epsilon
	);
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
		"wrong lightness for blue",
		1.0, hsl_result.z, epsilon
	);
	rgb_result = hsl2rgb(hsl_result);
	AssertEqual(
		"bad HSL to RGB conversion for blue",
		rgb_blue, rgb_result, epsilon
	);
}

}
}
}
