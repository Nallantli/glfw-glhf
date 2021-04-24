GCC=g++

CV=--std=c++17
CC=$(GCC) -Wall $(CV) -O2

LIBS=-lglfw3 -lopengl32 -lglew32 -lmingw32 -lSDL2main -lSDL2

gen.exe: main.o mesh.o gen.o QuickHull.o engine.o
	$(CC) -o $@ main.o mesh.o gen.o QuickHull.o engine.o $(LIBS)

main.o: main.cpp
	$(CC) -o $@ main.cpp -c $(LIBS)

engine.o: engine.cpp
	$(CC) -o $@ engine.cpp -c $(LIBS)

mesh.o: mesh.cpp
	$(CC) -o $@ mesh.cpp -c $(LIBS)

gen.o: gen.cpp
	$(CC) -o $@ gen.cpp -c $(LIBS)

QuickHull.o: quickhull/QuickHull.cpp
	$(CC) -o $@ quickhull/QuickHull.cpp -c $(LIBS)