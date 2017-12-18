#include "StringTest.hpp"

#include "math/const.hpp"
#include "ui/string.hpp"

CPPUNIT_TEST_SUITE_REGISTRATION(blobs::ui::test::StringTest);


namespace blobs {
namespace ui {
namespace test {

void StringTest::setUp() {
}

void StringTest::tearDown() {
}


void StringTest::testAngle() {
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"bad format of 0° angle",
		std::string("0.00°"), AngleString(0.0)
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"bad format of 30° angle",
		std::string("30.00°"), AngleString(PI * (1.0 / 6.0))
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"bad format of 45° angle",
		std::string("45.00°"), AngleString(PI * 0.25)
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"bad format of 60° angle",
		std::string("60.00°"), AngleString(PI * (1.0 / 3.0))
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"bad format of 90° angle",
		std::string("90.00°"), AngleString(PI * 0.5)
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"bad format of 180° angle",
		std::string("180.00°"), AngleString(PI)
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"bad format of -90° angle",
		std::string("-90.00°"), AngleString(PI * -0.5)
	);
}

void StringTest::testDecimal() {
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"bad format of zero places decimal string",
		std::string("0"), DecimalString(0.0, 0)
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"bad format of single place decimal string",
		std::string("1.2"), DecimalString(1.2, 1)
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"bad format of negative decimal string",
		std::string("-1.20"), DecimalString(-1.2, 2)
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"bad format of long decimal string",
		std::string("3141.593"), DecimalString(PI * 1000, 3)
	);
}

void StringTest::testLength() {
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"bad format of meter string",
		std::string("1.500m"), LengthString(1.5)
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"bad format of centimeter string",
		std::string("23.00cm"), LengthString(0.23)
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"bad format of millimeter string",
		std::string("0.7000mm"), LengthString(0.0007)
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"bad format of kilometer string",
		std::string("2.560km"), LengthString(2560.0)
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"bad format of megameter string",
		std::string("6.371Mm"), LengthString(6371000.0)
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"bad format of gigameter string",
		std::string("147.2Gm"), LengthString(147200000000.0)
	);
}

void StringTest::testMass() {
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"bad format of kilogram string",
		std::string("80.00kg"), MassString(80.0)
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"bad format of gram string",
		std::string("250.0g"), MassString(0.250)
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"bad format of milligram string",
		std::string("30.00mg"), MassString(0.000030)
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"bad format of ton string",
		std::string("18.00t"), MassString(18000.0)
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"bad format of gigaton string",
		std::string("50.00Gt"), MassString(50000000000000.0)
	);
}

void StringTest::testNumber() {
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"bad format of number string",
		std::string("0"), NumberString(0)
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"bad format of negative number string",
		std::string("-1500"), NumberString(-1500)
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"bad format of large number string",
		std::string("10000000"), NumberString(10000000)
	);
}

void StringTest::testPercentage() {
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"bad format of percentage string",
		std::string("0.0%"), PercentageString(0.0)
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"bad format of percentage string",
		std::string("50.0%"), PercentageString(0.5)
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"bad format of percentage string",
		std::string("150.2%"), PercentageString(1.502)
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"bad format of negative percentage string",
		std::string("-10.0%"), PercentageString(-0.1)
	);
}

void StringTest::testTime() {
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"bad format of seconds string",
		std::string("0s"), TimeString(0.0)
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"bad format of seconds string",
		std::string("10s"), TimeString(10.0)
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"bad format of minutes string",
		std::string("1m 00s"), TimeString(60.0)
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"bad format of minutes string",
		std::string("2m 05s"), TimeString(2.0 * 60.0 + 5.0)
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"bad format of minutes string",
		std::string("15m 20s"), TimeString(15.0 * 60.0 + 20.0)
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"bad format of minutes string",
		std::string("10h 05m 49s"), TimeString(10.0 * 60.0 * 60.0 + 5.0 * 60.0 + 49.0)
	);
}

void StringTest::testVector() {
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"bad format of vector string",
		std::string("<0.00, 0.00, 0.00>"), VectorString(glm::dvec3(0.0), 2)
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"bad format of vector string",
		std::string("<1.0, -2.5, 1.0>"), VectorString(glm::dvec3(1.0, -2.5, 0.96), 1)
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"bad format of vector string",
		std::string("<0, 0>"), VectorString(glm::ivec2(0))
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"bad format of vector string",
		std::string("<-3, 4>"), VectorString(glm::ivec2(-3, 4))
	);
}

}
}
}
