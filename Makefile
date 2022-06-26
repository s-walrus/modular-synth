prep:
	mkdir -p build

scratch: prep
	clang++ -g -std=c++20 -Wall -D__LINUX_PULSE__ -o build/scratch -lpthread -lpulse-simple -lpulse lib/RtAudio.cpp src/scratch.cpp

run:
	build/scratch

unit:
	clang++ src/unit-scratch.cpp -g -std=c++20 -o build/scratch

all: build run
