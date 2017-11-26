#include "StabilityTest.hpp"

#include "math/GaloisLFSR.hpp"
#include "math/SimplexNoise.hpp"
#include "math/WorleyNoise.hpp"

#include <cstdint>
#include <string>
#include <sstream>
#include <glm/gtx/io.hpp>

CPPUNIT_TEST_SUITE_REGISTRATION(blobs::math::test::StabilityTest);

using namespace std;


namespace blobs {
namespace math {
namespace test {

void StabilityTest::setUp() {

}

void StabilityTest::tearDown() {

}


void StabilityTest::testRNG() {
	GaloisLFSR random(0);
	uint16_t value;
	random(value);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"unexpected value #1 from RNG",
		uint16_t(0x0000), value
	);
	random(value);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"unexpected value #2 from RNG",
		uint16_t(0x0000), value
	);
	random(value);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"unexpected value #3 from RNG",
		uint16_t(0xB000), value
	);
	random(value);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"unexpected value #4 from RNG",
		uint16_t(0x0000), value
	);
	random(value);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"unexpected value #5 from RNG",
		uint16_t(0x0000), value
	);
	random(value);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"unexpected value #6 from RNG",
		uint16_t(0x0000), value
	);
	random(value);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"unexpected value #7 from RNG",
		uint16_t(0x4500), value
	);
	random(value);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"unexpected value #8 from RNG",
		uint16_t(0x0000), value
	);
	random(value);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"unexpected value #9 from RNG",
		uint16_t(0x0000), value
	);
	random(value);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"unexpected value #10 from RNG",
		uint16_t(0x0000), value
	);
	random(value);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"unexpected value #11 from RNG",
		uint16_t(0x2E70), value
	);
	random(value);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"unexpected value #12 from RNG",
		uint16_t(0x0000), value
	);
	random(value);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"unexpected value #13 from RNG",
		uint16_t(0x0000), value
	);
	random(value);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"unexpected value #14 from RNG",
		uint16_t(0x0000), value
	);
	random(value);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"unexpected value #15 from RNG",
		uint16_t(0x1011), value
	);
	random(value);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"unexpected value #16 from RNG",
		uint16_t(0x0000), value
	);
	random(value);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"unexpected value #17 from RNG",
		uint16_t(0x0000), value
	);
	random(value);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"unexpected value #18 from RNG",
		uint16_t(0xB000), value
	);
	value = random.Next<uint16_t>();
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"unexpected value #19 from RNG (using Next())",
		uint16_t(0x0B0B), value
	);
	random(value);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"unexpected value #20 from RNG",
		uint16_t(0x0000), value
	);
	random(value);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"unexpected value #21 from RNG",
		uint16_t(0x0000), value
	);
	random(value);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"unexpected value #22 from RNG",
		uint16_t(0x1500), value
	);
	random(value);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"unexpected value #23 from RNG",
		uint16_t(0x0454), value
	);
	random(value);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"unexpected value #24 from RNG",
		uint16_t(0x0000), value
	);
	random(value);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"unexpected value #25 from RNG",
		uint16_t(0x0000), value
	);
	value = random.Next<uint16_t>();
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"unexpected value #26 from RNG (using Next())",
		uint16_t(0xC970), value
	);
	random(value);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"unexpected value #27 from RNG",
		uint16_t(0x02E5), value
	);
	random(value);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"unexpected value #28 from RNG",
		uint16_t(0x0000), value
	);
	random(value);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"unexpected value #29 from RNG",
		uint16_t(0x0000), value
	);
	random(value);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"unexpected value #30 from RNG",
		uint16_t(0x0101), value
	);
	random(value);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"unexpected value #31 from RNG",
		uint16_t(0x0100), value
	);
	random(value);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"unexpected value #32 from RNG",
		uint16_t(0x0000), value
	);

	GaloisLFSR random1(1);
	uint16_t value1;
	for (int i = 0; i < 32; ++i) {
		random1(value1);
	}
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"RNG with seeds 0 and 1 differ",
		value, value1
	);

	GaloisLFSR random_bool(0);
	bool value_bool;
	for (int i = 0; i < (16 * 32); ++i) {
		random_bool(value_bool);
	}
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"unexpected result for bool",
		false, value_bool
	);

	GaloisLFSR random8(0);
	uint8_t value8;
	for (int i = 0; i < 31; ++i) {
		random8(value8);
	}
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"unexpected result for uint8",
		uint8_t(0x10), value8
	);

	GaloisLFSR random32(0);
	uint32_t value32;
	for (int i = 0; i < 16; ++i) {
		random32(value32);
	}
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"unexpected result for uint32",
		uint32_t(0xB0000000), value32
	);
}

void StabilityTest::testSimplex() {
	SimplexNoise noise(0);

	Assert(noise, glm::vec3(0.0f, 0.0f, 0.0f),  0.0f);
	Assert(noise, glm::vec3(0.0f, 0.0f, 1.0f),  0.652221322059631f);
	Assert(noise, glm::vec3(0.0f, 1.0f, 0.0f),  0.867977976799011f);
	Assert(noise, glm::vec3(0.0f, 1.0f, 1.0f), -0.107878111302853f);
	Assert(noise, glm::vec3(1.0f, 0.0f, 0.0f), -0.107878260314465f);
	Assert(noise, glm::vec3(1.0f, 0.0f, 1.0f), -6.31356940061778e-08f);
	Assert(noise, glm::vec3(1.0f, 1.0f, 0.0f), -0.107878245413303f);
	Assert(noise, glm::vec3(1.0f, 1.0f, 1.0f),  0.0f);

	Assert(noise, glm::vec3( 0.0f,  0.0f, -1.0f), -0.107878483831882f);
	Assert(noise, glm::vec3( 0.0f, -1.0f,  0.0f), -0.760099768638611f);
	Assert(noise, glm::vec3( 0.0f, -1.0f, -1.0f),  0.0f);
	Assert(noise, glm::vec3(-1.0f,  0.0f,  0.0f),  0.760099768638611f);
	Assert(noise, glm::vec3(-1.0f,  0.0f, -1.0f),  0.0f);
	Assert(noise, glm::vec3(-1.0f, -1.0f,  0.0f), -0.107878118753433f);
	Assert(noise, glm::vec3(-1.0f, -1.0f, -1.0f),  0.0f);
}

void StabilityTest::testWorley() {
	WorleyNoise noise(0);

	Assert(noise, glm::vec3(0.0f, 0.0f, 0.0f), -0.117765009403229f);
	Assert(noise, glm::vec3(0.0f, 0.0f, 1.0f), -0.209876894950867f);
	Assert(noise, glm::vec3(0.0f, 1.0f, 0.0f), -0.290086328983307f);
	Assert(noise, glm::vec3(0.0f, 1.0f, 1.0f), -0.332393705844879f);
	Assert(noise, glm::vec3(1.0f, 0.0f, 0.0f), -0.621925830841064f);
	Assert(noise, glm::vec3(1.0f, 0.0f, 1.0f), -0.338455379009247f);
	Assert(noise, glm::vec3(1.0f, 1.0f, 0.0f), -0.386664032936096f);
	Assert(noise, glm::vec3(1.0f, 1.0f, 1.0f), -0.533940434455872f);

	Assert(noise, glm::vec3( 0.0f,  0.0f, -1.0f), -0.425480604171753f);
	Assert(noise, glm::vec3( 0.0f, -1.0f,  0.0f), -0.189745843410492f);
	Assert(noise, glm::vec3( 0.0f, -1.0f, -1.0f), -0.30408102273941f);
	Assert(noise, glm::vec3(-1.0f,  0.0f,  0.0f), -0.618566155433655f);
	Assert(noise, glm::vec3(-1.0f,  0.0f, -1.0f), -0.060045599937439f);
	Assert(noise, glm::vec3(-1.0f, -1.0f,  0.0f), -0.366827547550201f);
	Assert(noise, glm::vec3(-1.0f, -1.0f, -1.0f), -0.575981974601746f);
}

void StabilityTest::Assert(
	const SimplexNoise &noise,
	const glm::vec3 &position,
	float expected
) {
	stringstream msg;
	msg << "unexpected simplex noise value at " << position;
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
		msg.str(),
		expected, noise(position), numeric_limits<float>::epsilon()
	);
}

void StabilityTest::Assert(
	const WorleyNoise &noise,
	const glm::vec3 &position,
	float expected
) {
	stringstream msg;
	msg << "unexpected worley noise value at " << position;
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
		msg.str(),
		expected, noise(position), numeric_limits<float>::epsilon()
	);
}

}
}
}
