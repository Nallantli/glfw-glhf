#include "engine/engine.h"

int main(int argc, char **argv)
{
	engine_t *engine;
	if (argc != 2) {
		engine = new engine_t(time(0));
	} else {
		engine = new engine_t(std::stoll(argv[1]));
	}

	engine->run();
	delete engine;

	return 0;
}