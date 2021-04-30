GCC=g++

CV=--std=c++17
CC=$(GCC) -Wall $(CV) -Os

LIBS=-lglfw3 -lopengl32 -lglew32 -lmingw32 -lSDL2main -lSDL2

gen.exe: main.o engine.o point3.o generator.o polar.o surface.o QuickHull.o shader.o SimplexNoise.o
	$(CC) -o $@ main.o engine.o point3.o generator.o polar.o surface.o QuickHull.o shader.o SimplexNoise.o $(LIBS)

main.o: main.cpp
	$(CC) -o $@ main.cpp -c $(LIBS)

engine.o: engine/engine.cpp
	$(CC) -o $@ engine/engine.cpp -c $(LIBS)

point3.o: point3/point3.cpp
	$(CC) -o $@ point3/point3.cpp -c $(LIBS)

generator.o: generator/generator.cpp
	$(CC) -o $@ generator/generator.cpp -c $(LIBS)

polar.o: polar/polar.cpp
	$(CC) -o $@ polar/polar.cpp -c $(LIBS)

surface.o: surface/surface.cpp
	$(CC) -o $@ surface/surface.cpp -c $(LIBS)

shader.o: shader/shader.cpp
	$(CC) -o $@ shader/shader.cpp -c $(LIBS)

QuickHull.o: quickhull/QuickHull.cpp
	$(CC) -o $@ quickhull/QuickHull.cpp -c $(LIBS)

SimplexNoise.o: SimplexNoise/SimplexNoise.cpp
	$(CC) -o $@ SimplexNoise/SimplexNoise.cpp -c $(LIBS)