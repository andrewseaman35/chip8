#include <iostream>
#include <SDL2/SDL.h>

#include "constants.h"
#include "chip8.h"
#include "chip8Window.h"

using namespace std;


int main(int argc, char *argv[]) {
    if (argc < 2) {
        cout << "ROM Path required!" << endl;
        cout << "Usage: ./chip8 <path/to/rom>" << endl;
        return 1;
    }

    Chip8 chip8 = Chip8();
    Chip8Window chip8Window = Chip8Window(&chip8, "Chip8", WINDOW_WIDTH, WINDOW_HEIGHT);
    if (!chip8.load(argv[1])) {
        return 1;
    }
    chip8Window.run();
}
