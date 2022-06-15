prep:
	mkdir -p build

build: prep
	g++ -Wall -D__LINUX_PULSE__ -o build/scratch -lpthread -lpulse-simple -lpulse lib/* src/*

run:
	build/scratch

all: build run
