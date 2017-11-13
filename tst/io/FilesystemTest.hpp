#ifndef BLOBS_TEST_IO_FILESYSTEMTEST_HPP
#define BLOBS_TEST_IO_FILESYSTEMTEST_HPP

#include <string>
#include <cppunit/extensions/HelperMacros.h>

namespace blobs {
namespace io {
namespace test {

class FilesystemTest
: public CppUnit::TestFixture {

CPPUNIT_TEST_SUITE(FilesystemTest);

CPPUNIT_TEST(testFile);
CPPUNIT_TEST(testDirectory);

CPPUNIT_TEST_SUITE_END();

public:
	void setUp();
	void tearDown();

	void testFile();
	void testDirectory();

private:
	std::string test_dir;

};

}
}
}

#endif
