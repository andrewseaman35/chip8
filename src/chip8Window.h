#ifndef CHIP_8_WINDOW_H
#define CHIP_8_WINDOW_H

#include <SDL2/SDL.h>

#include "chip8.h"

class Chip8Window {
private:
    Chip8* chip8;
    SDL_Window* window;
    SDL_Renderer* renderer;

    void initWindow(const char *title, int width, int height);

public:
    Chip8Window(Chip8* _chip8, const char *title, int width, int height);
    ~Chip8Window();

    void run();
};

#endif // CHIP_8_WINDOW_H
