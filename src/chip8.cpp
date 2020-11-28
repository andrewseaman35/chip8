#include <iostream>
#include <fstream>
#include <sys/stat.h>

#include "chip8.h"

using namespace std;

const int MEMORY_SIZE = 4096;
const int INTERPRETER_SIZE = 512;

const int DISPLAY_WIDTH = 64;
const int DISPLAY_HEIGHT = 32;


void Chip8::init() {
    opcode = 0;
    I = 0;
    sp = 0;
    delayTimer = 0;
    soundTimer = 0;
    pc = INTERPRETER_SIZE;

    this->clearDisplay();
    this->clearStack();
    this->clearRegisters();
    this->clearKeypad();

    std::cout << "Initialized\n";
}

void Chip8::clearDisplay() {
    for (int i = 0; i < (DISPLAY_WIDTH * DISPLAY_HEIGHT); i++) {
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

void Chip8::printDisplay() {
    cout << "printDisplay\n";
    unsigned short val;
    for (int j = 0; j < DISPLAY_HEIGHT; j ++) {
        for (int i = 0; i < DISPLAY_WIDTH; i++) {
            if (displayBuffer[i + (j * DISPLAY_WIDTH)] == 1) {
                cout << 'X';
            } else {
                cout << " ";
            }
        }
        cout << "\n";
    }
}

void Chip8::printRegisters() {
    cout << "printRegisters: \n   ";
    for (int i = 0; i < 16; i++) {
        cout << to_string(V[i]) << " ";
    }
    cout << "\n";
}

void Chip8::printStack() {
    cout << "printStack: \n   ";
    for (int i = 0; i < 16; i++) {
        cout << to_string(stack[i]) << " ";
    }
    cout << "\n";
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

    cout << "ROM loaded into memory!\n";
}

void Chip8::cycle() {
    // Fetch Opcode
    // opcode is two bytes long and located at the program counter
    // shift the first byte by 8 and OR it with the following byte
    opcode = memory[pc] << 8 | memory[pc + 1];
    cout << "\nOPCODE: " << hex << opcode;

    // Decode Opcode
    switch(opcode & 0xF000) {
        case 0x0000:
            switch(opcode & 0x00FF) {
                case 0x00E0:
                    // 00E0 - CLS
                    // Clear the display.
                    cout << " -- 00E0\n";
                    this->clearDisplay();
                    pc += 2;
                    break;
                case 0x00EE:
                    // 00EE - RET
                    // Return from a subroutine.
                    cout << " -- 00EE\n";
                    pc = stack[--sp];
                    cout << "Removed from stack: " << to_string(pc) << "\n";
                    pc += 2;
                    break;
                case 0x00C0:
                    // 00Cn - SCD nibble (super chip-48)
                    cout << "unhandled " << hex << opcode;
                    throw;
                case 0x00F0:
                    switch(opcode & 0x000F) {
                        // case 0x00B:
                            // 00FB - SCR (super chip-48)
                        // case 0x00C:
                            // 00FC - SCL (super chip-48)
                        // case 0x00D:
                            // 00FD - EXIT (super chip-48)
                        // case 0x00E:
                            // 00FE - LOW (super chip-48)
                        // case 0x00F:
                            // 00FF - HIGH (super chip-49)
                        default:
                            cout << "Unhandled " << hex << opcode << "\n";
                            throw 1;
                    }
                default:
                    // 0nnn - SYS addr (unnecessary, I think?)
                    cout << "Unhandled 0nnn instruction";
                    throw "Unhandled 0nnn instruction";
            }
            break;
        case 0x1000:
            // 1nnn - JP addr
            // Jump to location nnn.
            cout << " -- 1nnn\n";
            pc = opcode & 0x0FFF;
            cout << "Jumping to " << to_string(pc) << "\n";
            break;
        case 0x2000:
            // 2nnn - CALL addr
            // Call subroutine at nnn.
            cout << " -- 2nnn\n";
            stack[sp++] = pc;
            cout << "Added to stack: " << to_string(pc) << "\n";
            pc = opcode & 0x0FFF;
            this->printStack();
            break;
        case 0x3000:
            // 3xkk - SE Vx, byte
            // Skip next instruction if Vx = kk.
            cout << " -- 3xkk\n";
            pc += V[(opcode & 0x0F00) >> 8] == (opcode & 0x00FF) ? 4 : 2;
            break;
        case 0x4000:
            // 4xkk - SNE Vx, byte
            // Skip next instruction if Vx != kk.
            cout << " -- 4xkk\n";
            pc += V[(opcode & 0x0F00) >> 8] != (opcode & 0x00FF) ? 4 : 2;
            break;
        case 0x5000:
            // 5xy0 - SE Vx, Vy
            // Skip next instruction if Vx = Vy.
            cout << " -- 5xk0\n";
            pc += V[(opcode & 0x0F00) >> 8] == V[(opcode & 0x00F) >> 4] ? 4 : 2;
            break;
        case 0x6000:
            // 6xkk - LD Vx, byte
            // Set Vx = kk.
            cout << " -- 6xkk\n";
            V[(opcode & 0x0F00) >> 8] = (opcode & 0x00FF);
            pc += 2;
            this->printRegisters();
            break;
        case 0x7000:
            // 7xkk - ADD Vx, byte
            // Set Vx = Vx + kk.
            cout << " -- 7xkk\n";
            V[(opcode & 0x0F00) >> 8] += (opcode & 0x00FF);
            pc += 2;
            break;
        case 0x8000:
            switch(opcode & 0x000F) {
                case 0x0000:
                    // 8xy0 - LD Vx, Vy
                    // Set Vx = Vy.
                    cout << " -- 8xy0\n";
                    V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4];
                    pc += 2;
                    break;
                case 0x0001:
                    // 8xy1 - OR Vx, Vy
                    // Set Vx = Vx OR Vy.
                    cout << " -- 8xy1\n";
                    V[(opcode & 0x0F00) >> 8] |= V[(opcode & 0x00F0) >> 4];
                    pc += 2;
                    break;
                case 0x0002:
                    // 8xy2 - AND Vx, Vy
                    // Set Vx = Vx AND Vy.
                    cout << " -- 8xy2\n";
                    V[(opcode & 0x0F00) >> 8] &= V[(opcode & 0x00F0) >> 4];
                    pc += 2;
                    break;
                case 0x0003:
                    // 8xy3 - OR Vx, Vy
                    // Set Vx = Vx XOR Vy.
                    cout << " -- 8xy2\n";
                    V[(opcode & 0x0F00) >> 8] ^= V[(opcode & 0x00F0) >> 4];
                    pc += 2;
                    break;
                case 0x0004:
                    // 8xy4 - ADD Vx, Vy
                    // Set Vx = Vx + Vy, set VF = carry.
                    V[(opcode & 0x0F00) >> 8] += V[(opcode & 0x00F0) >> 4];
                    V[0xF] = V[(opcode & 0x0F00) >> 8] > 0xFF ? 1 : 0;
                    pc += 2;
                    break;
                case 0x0005:
                    // 8xy5 - SUB Vx, Vy
                    // Set Vx = Vx - Vy, set VF = NOT borrow.
                    V[0xF] = V[(opcode & 0x0F00) >> 8] > V[(opcode & 0x00F0) >> 4] ? 1 : 0;
                    V[(opcode & 0x0F00) >> 8] -= V[(opcode & 0x00F0) >> 4];
                    pc += 2;
                    break;
                case 0x0006:
                    // 8xy6 - SHR Vx {, Vy}
                    cout << "Unhandled " << hex << opcode << "\n";
                    throw 1;
                case 0x0007:
                    // 8xy7 - SUBN Vy, Vy
                    cout << "Unhandled " << hex << opcode << "\n";
                    throw 1;
                case 0x000E:
                    // 8xyE - SHL Vx {, Vy}
                    // Set Vx = Vx SHL 1.
                    // If the most-significant bit of Vx is 1, then VF is set to 1, otherwise to 0.
                    V[0xF] = V[(opcode & 0x0F00) >> 8] >> 7;
                    V[(opcode & 0x0F00) >> 8] <<= 1;
                    pc += 2;
                    break;
                default:
                    cout << "Unhandled " << hex << opcode << "\n";
                    throw 1;
            }
            break;
        case 0x9000:
            // 9xy0 - SNE Vx, Vy
            // Skip next instruction if Vx != Vy.
            cout << " -- 9xy0\n";
            pc += V[(opcode & 0x0F00) >> 8] != V[(opcode & 0x00F0) >> 4] ? 4 : 2;
            break;
        case 0xA000:
            // Annn - LD I, addr
            // Set I = nnn.
            cout << " -- Annn\n";
            I = opcode & 0x0FFF;
            pc += 2;
            break;
        case 0xB000:
            // Bnnn - JP V0, addr
            cout << "Unhandled " << hex << opcode << "\n";
            throw 2;
        case 0xC000:
            // Cxkk - RND Vx, byte
            cout << "Unhandled " << hex << opcode << "\n";
            throw;
        case 0xD000:
        {
            // Dxyn - DRW Vx, Vy, nibble
            // Display n-byte sprite starting at memory location I at (Vx, Vy), set VF = collision.
            cout << " -- Dxyn\n";
            unsigned short xStart = V[(opcode & 0x0F00) >> 8];
            unsigned short yStart = V[(opcode & 0x00F0) >> 4];
            unsigned short height = opcode & 0x000F;

            // If this causes any pixels to be erased, VF is set to 1, otherwise it is set to 0.
            V[0xF] = 0;

            unsigned short pos;
            unsigned short val;
            for (int y = 0; y < height; y++) {
                // The interpreter reads n bytes from memory, starting at the address stored in I
                val = memory[I + y];
                for (int x = 0; x < 8; x++) {
                    if((val & (0x80 >> x)) != 0) {
                        pos = (xStart + x + ((yStart + y) * DISPLAY_WIDTH));
                        if (displayBuffer[pos] == 1) {
                            // If this causes any pixels to be erased, VF is set to 1
                            V[0xF] = 1;
                        }

                        // Sprites are XORed onto the existing screen
                        displayBuffer[pos] ^= 1;
                    }
                }
            }
            pc += 2;

            this->printDisplay();
            break;
        }
        case 0xE000:
            switch(opcode & 0x00FF) {
                // case 0x009E:
                    // Ex9E - SKP Vx
                // case 0x00A1:
                    // ExA1 - SKNP Vx
                default:
                    cout << "Unhandled " << hex << opcode << "\n";
                    throw 2;
            }
        case 0xF000:
            switch(opcode & 0x00FF) {
                // case 0x0007:
                    // Fx07 - LD Vx, DT
                // case 0x000A:
                    // Fx0A - LD Vx, K
                // case 0x0015:
                    // Fx15: - LD DT, Vx
                // case 0x0018:
                    // Fx18 - LD ST, Vx
                // case 0x001E:
                    // Fx1E - ADD I, Vx
                // case 0x0029:
                    // Fx29 - LD F, Vx
                // case 0x0030:
                    // Fx30 - LD HF, Vx (super chip-48)
                // case 0x0033:
                    // Fx33 - LD B, Vx
                // case 0x0055:
                    // Fx55 - LD [I], Vx
                // case 0x0065:
                    // Fx65 - LD Vx, [I]
                // case 0x0075:
                    // Fx75 - LD R, Vx
                // case 0x85:
                    // Fx85 - LD Vx, R
                default:
                    cout << "Unhandled " << hex << opcode << "\n";
                    throw "asdf";
            }
            break;
    }

    // Execute Opcode

    // Update timers
}
