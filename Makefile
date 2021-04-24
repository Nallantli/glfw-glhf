GCC=g++

CV=--std=c++17
CC=$(GCC) -Wall $(CV) -O2

LIBS=-lglfw3 -lopengl32 -lglew32

gen.exe: main.o mesh.o QuickHull.o
	$(CC) -o $@ main.o mesh.o QuickHull.o $(LIBS)

main.o: main.cpp
	$(CC) -o $@ main.cpp -c $(LIBS)

mesh.o: mesh.cpp
	$(CC) -o $@ mesh.cpp -c $(LIBS)

QuickHull.o: quickhull/QuickHull.cpp
	$(CC) -o $@ quickhull/QuickHull.cpp -c $(LIBS)