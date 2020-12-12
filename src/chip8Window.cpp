#include <iostream>
#include <chrono>
#include <unordered_map>
#include <thread>
#include <SDL2/SDL.h>

#include "chip8Window.h"
#include "constants.h"
#include "logger.h"


using namespace std;


Logger* logger2 = Logger::getLogger();


std::unordered_map<int, int> KEYMAP = {
    { SDLK_1, 0 },
    { SDLK_2, 1 },
    { SDLK_3, 2 },
    { SDLK_4, 3 },
    { SDLK_q, 4 },
    { SDLK_w, 5 },
    { SDLK_e, 6 },
    { SDLK_r, 7 },
    { SDLK_a, 8 },
    { SDLK_s, 9 },
    { SDLK_d, 10 },
    { SDLK_f, 11 },
    { SDLK_z, 12 },
    { SDLK_x, 13 },
    { SDLK_c, 14 },
    { SDLK_v, 15 }
};


Chip8Window::Chip8Window(Chip8* _chip8, const char *title, int width, int height) {
    if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO) < 0) {
        cout << "SDL_Init failure: " << SDL_GetError() << endl;
        exit(1);
    }
    cout << "SDL_Init success!\n";

    chip8 = _chip8;

    this->initWindow(title, width, height);
}

Chip8Window::~Chip8Window() {
    if (window != NULL) {
        SDL_DestroyWindow(window);
    }
    SDL_Quit();
}

void Chip8Window::initWindow(const char *title, int width, int height) {
    window = SDL_CreateWindow(
        title,
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        width,
        height,
        SDL_WINDOW_SHOWN);

    if (window == NULL){
        cout << "SDL_CreateWindow failure: " << SDL_GetError() << endl;
        exit(2);
    }
    cout << "SDL_CreateWindow success!\n";

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (renderer == nullptr){
        SDL_DestroyWindow(window);
        cout << "SDL_CreateRenderer Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        exit(1);
    }

    cout << "SDL_CreateRenderer success!\n";
}

void Chip8Window::run() {
    SDL_Event e;

    SDL_Texture* sdlTexture = SDL_CreateTexture(renderer,
        // RGBA, where each value is one byte
        SDL_PIXELFORMAT_RGBA8888,

        // https://wiki.libsdl.org/SDL_TextureAccess - "changes frequently, lockable"
        SDL_TEXTUREACCESS_STREAMING,
        DISPLAY_WIDTH,
        DISPLAY_HEIGHT);

    uint32_t sdlTextureBuffer[DISPLAY_WIDTH * DISPLAY_HEIGHT];

    bool quit = false;
    // int quit = 2;
    while (!quit) {
        while (SDL_PollEvent(&e)){
            if (e.type == SDL_QUIT){
                quit = true;
            }
            if (e.type == SDL_KEYDOWN){
                if (e.key.keysym.sym == SDLK_ESCAPE) {
                    quit = true;
                } else if (KEYMAP.count(e.key.keysym.sym)) {
                    chip8->handleKeyDown(KEYMAP[e.key.keysym.sym]);
                    logger2->debug(chip8->keypadToString());
                }
            } else if (e.type == SDL_KEYUP) {
                if (KEYMAP.count(e.key.keysym.sym)) {
                    chip8->handleKeyUp(KEYMAP[e.key.keysym.sym]);
                    logger2->debug(chip8->keypadToString());
                }
            } else if (e.type == SDL_MOUSEBUTTONDOWN){
                // quit = true;
            }
        }
        chip8->cycle();

        if (chip8->requiresRerender) {
            for (int i = 0; i < DISPLAY_WIDTH * DISPLAY_HEIGHT; ++i) {
                    uint8_t pixel = chip8->displayBuffer[i];
                    sdlTextureBuffer[i] = (PIXEL_COLOR * pixel) | PIXEL_ALPHA;
                }
            // Update SDL texture
            SDL_UpdateTexture(sdlTexture, NULL, sdlTextureBuffer, 64 * sizeof(Uint32));
            // Clear screen and render
            SDL_RenderClear(renderer);
            SDL_RenderCopy(renderer, sdlTexture, NULL, NULL);
            SDL_RenderPresent(renderer);
        }

        // this_thread::sleep_for(chrono::microseconds(MICROSECOND_DELAY));
    }
}
