prep:
	mkdir -p build

examples/%.cpp: prep
	clang++ lib/miniaudio.c src/player.cpp src/synth.cpp $@ -g -std=c++20 -o build/run

run:
	build/run

all: build run
