#ifndef BLOBS_TEST_ASSETS_HPP_
#define BLOBS_TEST_ASSETS_HPP_

#include "math/glm.hpp"

#include <string>
#include <limits>


namespace blobs {
namespace test {

template<class T>
void AssertEqual(
	const std::string &msg,
	const glm::tvec2<T> &expected,
	const glm::tvec2<T> &actual,
	T epsilon = std::numeric_limits<T>::epsilon()
) {
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
		msg + " (X component)",
		expected.x, actual.x, epsilon
	);
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
		msg + " (Y component)",
		expected.y, actual.y, epsilon
	);
}

template<class T>
void AssertEqual(
	const std::string &msg,
	const glm::tvec3<T> &expected,
	const glm::tvec3<T> &actual,
	T epsilon = std::numeric_limits<T>::epsilon()
) {
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
		msg + " (X component)",
		expected.x, actual.x, epsilon
	);
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
		msg + " (Y component)",
		expected.y, actual.y, epsilon
	);
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
		msg + " (Z component)",
		expected.z, actual.z, epsilon
	);
}

}
}

#endif
