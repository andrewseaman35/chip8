#include <iostream>
#include <SDL2/SDL.h>

#include "constants.h"
#include "chip8.h"
#include "chip8Window.h"

using namespace std;


int main() {
    Chip8 chip8 = Chip8();

    Chip8Window chip8Window = Chip8Window(&chip8, "Chip8", WINDOW_WIDTH, WINDOW_HEIGHT);

    chip8.load("./roms/pong.ch8");

    chip8Window.run();
}
