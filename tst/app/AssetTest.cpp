#include "AssetTest.hpp"

#include "app/Assets.hpp"
#include "app/init.hpp"

CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(blobs::app::test::AssetTest, "headed");


namespace blobs {
namespace app {
namespace test {

void AssetTest::setUp() {
}

void AssetTest::tearDown() {
}


void AssetTest::testLoadAll() {
	Init init(true, 8);
	Assets assets;

	CPPUNIT_ASSERT_MESSAGE(
		"no resources loaded",
		assets.data.resources.Size() > 0
	);
	CPPUNIT_ASSERT_MESSAGE(
		"no tile types loaded",
		assets.data.resources.Size() > 0
	);

	CPPUNIT_ASSERT_MESSAGE(
		"tile texture has no width",
		assets.textures.tiles.Width() > 0
	);
	CPPUNIT_ASSERT_MESSAGE(
		"tile texture has no height",
		assets.textures.tiles.Height() > 0
	);
	CPPUNIT_ASSERT_MESSAGE(
		"tile texture has no depth",
		assets.textures.tiles.Depth() > 0
	);

	CPPUNIT_ASSERT_MESSAGE(
		"skin texture has no width",
		assets.textures.skins.Width() > 0
	);
	CPPUNIT_ASSERT_MESSAGE(
		"skin texture has no height",
		assets.textures.skins.Height() > 0
	);
	CPPUNIT_ASSERT_MESSAGE(
		"skin texture has no depth",
		assets.textures.skins.Depth() > 0
	);

	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"large font has wrong family",
		std::string("DejaVu Sans"), std::string(assets.fonts.large.FamilyName())
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"medium font has wrong family",
		std::string("DejaVu Sans"), std::string(assets.fonts.medium.FamilyName())
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"small font has wrong family",
		std::string("DejaVu Sans"), std::string(assets.fonts.small.FamilyName())
	);
}

}
}
}
