prep:
	mkdir -p build

examples/%.cpp: prep
	clang++ lib/miniaudio.c src/player.cpp src/synth.cpp $@ -g -std=c++20 -o build/run

examples/%.su: prep
	python3 src/cli/unit-generator.py < $@ > build/_generated-su.h
	clang++ lib/miniaudio.c src/player.cpp src/synth.cpp examples/single-su-template.cpp -g -std=c++20 -o build/run
	rm build/_generated-su.h

run:
	build/run

all: build run
