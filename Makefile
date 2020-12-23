compile: src/main.cpp src/chip8.cpp
	g++ src/main.cpp src/chip8.cpp src/chip8Window.cpp src/logger.cpp -o chip8 $$(sdl2-config --cflags --libs) -std=c++11


test: test/testInstructions.cpp
	g++ test/testInstructions.cpp src/chip8.cpp src/chip8Window.cpp src/logger.cpp -o test_prog $$(sdl2-config --cflags --libs) -std=c++11
	./test_prog

.PHONY: compile test
