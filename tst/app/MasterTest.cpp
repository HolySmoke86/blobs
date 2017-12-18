#include "MasterTest.hpp"

#include "../event.hpp"

#include "app/Application.hpp"
#include "app/Assets.hpp"
#include "app/init.hpp"
#include "app/MasterState.hpp"
#include "creature/Creature.hpp"
#include "world/Planet.hpp"
#include "world/Simulation.hpp"

CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(blobs::app::test::MasterTest, "headed");

using blobs::test::FakeKeyPress;
using blobs::test::FakeMouseClick;
using blobs::test::FakeQuit;


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

void MasterTest::testBasicInteraction() {
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

	// skip first 200ms to let camera settle
	for (int t = 0; t < 200; t += 17) {
		app.Loop(17);
	}

	CPPUNIT_ASSERT_MESSAGE(
		"creature panel not shown",
		state.GetCreaturePanel().Shown()
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"creature panel showing wrong creature",
		const_cast<const creature::Creature *>(blob), &state.GetCreaturePanel().GetCreature()
	);
	CPPUNIT_ASSERT_MESSAGE(
		"records panel not shown",
		state.GetRecordsPanel().Shown()
	);
	CPPUNIT_ASSERT_MESSAGE(
		"body panel shown",
		!state.GetBodyPanel().Shown()
	);

	// hide records panel
	FakeKeyPress(SDLK_F1);
	app.Loop(17);
	CPPUNIT_ASSERT_MESSAGE(
		"records panel shown",
		!state.GetRecordsPanel().Shown()
	);

	// show records panel
	FakeKeyPress(SDLK_F1);
	app.Loop(17);
	CPPUNIT_ASSERT_MESSAGE(
		"records panel not shown",
		state.GetRecordsPanel().Shown()
	);

	// click on blob
	FakeMouseClick(SDL_BUTTON_LEFT, init.viewport.Width() / 2, init.viewport.Height() / 2);
	app.Loop(17);
	CPPUNIT_ASSERT_MESSAGE(
		"creature panel not shown",
		state.GetCreaturePanel().Shown()
	);
	CPPUNIT_ASSERT_MESSAGE(
		"body panel shown",
		!state.GetBodyPanel().Shown()
	);

	// click on planet
	FakeMouseClick(SDL_BUTTON_LEFT, init.viewport.Width() / 3, init.viewport.Height() / 2);
	app.Loop(17);
	CPPUNIT_ASSERT_MESSAGE(
		"creature panel shown",
		!state.GetCreaturePanel().Shown()
	);
	CPPUNIT_ASSERT_MESSAGE(
		"body panel not shown",
		state.GetBodyPanel().Shown()
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"body panel showing wrong body",
		static_cast<const world::Body *>(&sim.PlanetByName("Planet")), &state.GetBodyPanel().GetBody()
	);

	// close window
	FakeQuit();
	app.Loop(17);
	CPPUNIT_ASSERT_MESSAGE(
		"app didn't exit after quit event",
		!app.HasState()
	);
}

}
}
}
