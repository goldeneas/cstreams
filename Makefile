.PHONY: build clean

build:
	gcc *.c -Wpedantic -o build/main

run: build
	./build/main
