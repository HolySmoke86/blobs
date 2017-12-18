#include "MasterTest.hpp"

#include "app/Application.hpp"
#include "app/Assets.hpp"
#include "app/init.hpp"
#include "app/MasterState.hpp"
#include "creature/Creature.hpp"
#include "world/Simulation.hpp"

CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(blobs::app::test::MasterTest, "headed");


namespace blobs {
namespace app {
namespace test {

void MasterTest::setUp() {
}

void MasterTest::tearDown() {
}


void MasterTest::testOneSecond() {
	Init init(false, 1);
	Assets assets;

	world::Simulation sim(assets);
	assets.LoadUniverse("universe", sim);

	auto blob = new creature::Creature(sim);
	blob->Name(assets.name.Sequential());
	Spawn(*blob, sim.PlanetByName("Planet"));
	// decrease chances of ur-blob dying without splitting
	blob->GetProperties().Fertility() = 1.0;
	blob->BuildVAO();

	app::MasterState state(assets, sim);
	state.Show(*blob);

	app::Application app(init.window, init.viewport);
	app.PushState(&state);

	// just run it for one second and check if anything segfaults or throws
	for (int t = 0; t < 1000; t += 17) {
		app.Loop(17);
	}
}

}
}
}
