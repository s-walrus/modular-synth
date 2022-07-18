prep:
	mkdir -p build

examples/%.cpp: prep
	clang++ -fconstexpr-steps=10000000 lib/miniaudio.c src/player.cpp src/synth.cpp $@ -g -std=c++20 -o build/run

examples/%.su: prep
	python3 src/cli/unit-generator.py < $@ > build/_generated-su.h
	clang++ -fconstexpr-steps=10000000 lib/miniaudio.c src/player.cpp src/synth.cpp examples/single-su-template.cpp -g -std=c++20 -o build/run
	rm build/_generated-su.h

benchmark: prep
	python3 src/cli/unit-generator.py < examples/sine.su > build/_generated-su.h
	clang++ -fconstexpr-steps=10000000 lib/miniaudio.c src/player.cpp src/synth.cpp examples/benchmark-template.cpp -DNDEBUG -O3 -std=c++20 -o build/run
	make run
	rm build/_generated-su.h

run:
	build/run

all: build run
