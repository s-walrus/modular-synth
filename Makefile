prep:
	mkdir -p build

scratch: prep
	g++ -Wall -D__LINUX_PULSE__ -o build/scratch -lpthread -lpulse-simple -lpulse lib/* src/*

run:
	build/scratch

unit:
	clang++ src/unit-scratch.cpp -g -std=c++20 -o build/scratch

all: build run
