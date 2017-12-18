#include "AssetTest.hpp"

#include "app/Assets.hpp"
#include "app/init.hpp"
#include "world/Simulation.hpp"

CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(blobs::app::test::AssetTest, "headed");


namespace blobs {
namespace app {
namespace test {

void AssetTest::setUp() {
}

void AssetTest::tearDown() {
}


void AssetTest::testLoadBasic() {
	Init init(false, 1);
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

void AssetTest::testLoadUniverse() {
	Init init(false, 1);
	Assets assets;

	world::Simulation sim(assets);
	assets.LoadUniverse("universe", sim);

	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"wrong number of suns in default universe",
		std::set<world::Sun *>::size_type(1), sim.Suns().size()
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"wrong number of planets in default universe",
		std::set<world::Planet *>::size_type(3), sim.Planets().size()
	);
	CPPUNIT_ASSERT_NO_THROW_MESSAGE(
		"spawn planet does not exist",
		sim.PlanetByName("Planet")
	);
}

}
}
}
