.PHONY: atom_d

all: build/atom_d build/atom_r

DEBUG_FLAGS = -Wall -Wunused -Wextra -Wno-write-strings -Wno-unused-parameter -g3 -ggdb -O0
RELEASE_FLAGS = -O2

CXX = g++

INCLUDE_DIRS = "src"
LIBS = -lSDL2main -lSDL2 -lm -lGL -lstdc++

ATOM_LINUX_SRC_UNIT_PATH = "src/atomorg/linux_atom.cpp"

build/linux_atom_d.o: src/atomorg/*.cpp src/*.h src/atomorg/*.h
	echo "Building Debug Build"
	mkdir -p build
	${CXX} ${DEBUG_FLAGS} -I${INCLUDE_DIRS} ${ATOM_LINUX_SRC_UNIT_PATH} \
		-c -o ./build/linux_atom_d.o

build/linux_atom_r.o: src/atomorg/*.cpp src/*.h src/atomorg/*.h
	echo "Building Release Build"
	mkdir -p build
	${CXX} ${RELEASE_FLAGS} -I${INCLUDE_DIRS} \
		-c ${ATOM_LINUX_SRC_UNIT_PATH} -o ./build/linux_atom_r.o

build/atom_d: build/linux_atom_d.o
	${CXX} -o build/atom_d build/linux_atom_d.o ${LIBS}

build/atom_r: build/linux_atom_r.o
	${CXX} -o build/atom_r build/linux_atom_r.o ${LIBS}

clean:
	rm -f build/linux_atom_d.o build/linux_atom_r.o build/atom_d build/atom_r
