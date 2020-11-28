#include <iostream>
#include <fstream>
#include <sys/stat.h>

#include "chip8.h"

using namespace std;

const int MEMORY_SIZE = 4096;
const int INTERPRETER_SIZE = 512;


void Chip8::init() {
    opcode = 0;
    I = 0;
    sp = 0;
    delayTimer = 0;
    soundTimer = 0;
    pc = 0;

    this->clearDisplay();
    this->clearStack();
    this->clearRegisters();
    this->clearKeypad();

    std::cout << "Initialized\n";
}

void Chip8::clearDisplay() {
    for (int i = 0; i < 2048; i++) {
        displayBuffer[i] = 0;
    }
}

void Chip8::clearStack() {
    for (int i = 0; i < 16; i++) {
        stack[i] = 0;
    }
}

void Chip8::clearRegisters() {
    for (int i = 0; i < 16; i++) {
        V[i] = 0;
    }
}

void Chip8::clearKeypad() {
    for (int i = 0; i < 16; i++) {
        keypad[i] = 0;
    }
}

void Chip8::load(const char *romPath) {
    cout << "Loading rom: " << romPath << "\n";

    // Get the file size in bytes
    struct stat fileStat;
    int romFileSize = 0;
    if (stat(romPath, &fileStat) == 0) {
        romFileSize = fileStat.st_size;
        cout << "ROM File size (bytes): " << romFileSize << "\n";
    } else {
        throw "Error while running stat";
    }

    if (romFileSize > (MEMORY_SIZE - INTERPRETER_SIZE)) {
        throw "ROM too big\n";
    }

    // Read the file into a buffer
    char romReadBuffer[romFileSize];
    ifstream romFile(romPath, ios::in | ios::binary);
    romFile.read(romReadBuffer, romFileSize);
    if (!romFile) {
        throw "Error reading ROM";
    }
    romFile.close();

    // Copy the file into memory, starting right after where the interpreter
    // would have lived
    for (int i = 0; i < romFileSize; i++) {
        memory[INTERPRETER_SIZE + i] = romReadBuffer[i];
    }

    cout << "ROM loaded into memory!";
}

void Chip8::cycle() {
    // Fetch Opcode
    // opcode is two bytes long and located at the program counter
    // shift the first byte by 8 and OR it with the following byte
    opcode = memory[pc] << 8 | memory[pc + 1];

    // Decode Opcode
    switch(opcode & 0xF000) {
        case 0x0000:
            switch(opcode & 0x00F0) {
                case 0x00E0:
                    // 00E0 - CLS
                    // Clear the display.
                    this->clearDisplay();
                    break;
                case 0x00EE:
                    // 00EE - RET
                    // Return from a subroutine.
                    break;
                case 0x00C0:
                    // 00Cn - SCD nibble (super chip-48)
                    break;
                case 0x00F0:
                    switch(opcode & 0x000F) {
                        case 0x00B:
                            // 00FB - SCR (super chip-48)
                            break;
                        case 0x00C:
                            // 00FC - SCL (super chip-48)
                            break;
                        case 0x00D:
                            // 00FD - EXIT (super chip-48)
                            break;
                        case 0x00E:
                            // 00FE - LOW (super chip-48)
                            break;
                        case 0x00F:
                            // 00FF - HIGH (super chip-49)
                            break;
                    }
                default:
                    // 0nnn - SYS addr (unnecessary, I think?)
                    break;
            }
        case 0x1000:
            // 1nnn - JP addr
            break;
        case 0x2000:
            // 2nnn - CALL addr
            break;
        case 0x3000:
            // 3xkk - SE Vx, byte
            break;
        case 0x4000:
            // 4xkk - SNE Vx, byte
            break;
        case 0x5000:
            // 5xy0 - SE Vx, Vy
            break;
        case 0x6000:
            // 6xkk - LD Vx, byte
            break;
        case 0x7000:
            // 7xkk - ADD Vx, byte
            break;
        case 0x8000:
            switch(opcode & 0x000F) {
                case 0x000:
                    // 8xy0 - LD Vx, Vy
                    break;
                case 0x001:
                    // 8xy1 - OR Vx, Vy
                    break;
                case 0x002:
                    // 8xy2 - AND Vx, Vy
                    break;
                case 0x003:
                    // 8xy3 - OR Vx, Vy
                    break;
                case 0x004:
                    // 8xy4 - ADD Vx, Vy
                    break;
                case 0x005:
                    // 8xy5 - SUB Vx, Vy
                    break;
                case 0x006:
                    // 8xy6 - SHR Vx {, Vy}
                    break;
                case 0x007:
                    // 8xy7 - SUBN Vy, Vy
                    break;
                case 0x00E:
                    // 8xyE - SHL Vx {, Vy}
                    break;
            }
        case 0x9000:
            // 9xy0 - SNE Vx, Vy
            break;
        case 0xA000:
            // Annn - LD I, addr
            break;
        case 0xB000:
            // Bnnn - JP V0, addr
            break;
        case 0xC000:
            // Cxkk - RND Vx, byte
            break;
        case 0xD000:
            // Dxyn - DRW Vx, Vy, nibble
            break;
        case 0xE000:
            switch(opcode & 0x00FF) {
                case 0x009E:
                    // Ex9E - SKP Vx
                    break;
                case 0x00A1:
                    // ExA1 - SKNP Vx
                    break;
            }
        case 0xF000:
            switch(opcode & 0x00FF) {
                case 0x0007:
                    // Fx07 - LD Vx, DT
                    break;
                case 0x000A:
                    // Fx0A - LD Vx, K
                    break;
                case 0x0015:
                    // Fx15: - LD DT, Vx
                    break;
                case 0x0018:
                    // Fx18 - LD ST, Vx
                    break;
                case 0x001E:
                    // Fx1E - ADD I, Vx
                    break;
                case 0x0029:
                    // Fx29 - LD F, Vx
                    break;
                case 0x0030:
                    // Fx30 - LD HF, Vx (super chip-48)
                    break;
                case 0x0033:
                    // Fx33 - LD B, Vx
                    break;
                case 0x0055:
                    // Fx55 - LD [I], Vx
                    break;
                case 0x0065:
                    // Fx65 - LD Vx, [I]
                    break;
                case 0x0075:
                    // Fx75 - LD R, Vx
                    break;
                case 0x85:
                    // Fx85 - LD Vx, R
                    break;
            }
    }

    // Execute Opcode

    // Update timers
}
