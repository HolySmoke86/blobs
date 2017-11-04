#include "app/Application.hpp"
#include "app/init.hpp"
#include "world/Planet.hpp"

#include <exception>
#include <iostream>


using namespace blobs;

int main(int argc, char *argv[]) {
	app::Init init;
	world::Planet planet(1); // r=1 should be a 3³

	app::Application app;
}
