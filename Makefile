compile: src/main.cpp src/chip8.cpp
	g++ src/main.cpp src/chip8.cpp src/chip8Window.cpp src/logger.cpp -o out $$(sdl2-config --cflags --libs) -std=c++11


run: compile
	./out
