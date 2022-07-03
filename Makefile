prep:
	mkdir -p build

scratch: prep
	clang++ lib/miniaudio.c src/player.cpp src/synth.cpp src/scratch.cpp -lpthread -g -std=c++20 -o build/scratch

run:
	build/scratch

all: build run
