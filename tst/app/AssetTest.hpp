#ifndef BLOBS_TEST_APP_ASSETTEST_HPP_
#define BLOBS_TEST_APP_ASSETTEST_HPP_

#include <cppunit/extensions/HelperMacros.h>


namespace blobs {
namespace app {
namespace test {

class AssetTest
: public CppUnit::TestFixture {

CPPUNIT_TEST_SUITE(AssetTest);

CPPUNIT_TEST(testLoadBasic);
CPPUNIT_TEST(testLoadUniverse);

CPPUNIT_TEST_SUITE_END();

public:
	void setUp();
	void tearDown();

	void testLoadBasic();
	void testLoadUniverse();

};

}
}
}

#endif
