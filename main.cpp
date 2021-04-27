#include "engine/engine.h"

int main(int argc, char **argv)
{
	if (argc != 2) {
		engine World(time(0));
		World.run();
	} else {
		engine World(std::stoll(argv[1]));
		World.run();
	}

	return 0;
}