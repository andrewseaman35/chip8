#include <iostream>

#include "chip8.h"

using namespace std;

int main() {
    Chip8 chip8 = Chip8();

    chip8.init();
    chip8.load("./roms/test_opcode.ch8");
}
