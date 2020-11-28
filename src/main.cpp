#include <iostream>

#include "chip8.h"

using namespace std;

int main() {
    Chip8 chip8 = Chip8();

    chip8.init();
    chip8.load("./roms/test_opcode.ch8");

    int LIMIT = 42;
    for (int i = 0; i < LIMIT; i++) {
        chip8.cycle();
    }
}
