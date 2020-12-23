#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include <chrono>

#include "logger.h"
#include "chip8.h"
#include "constants.h"

using namespace std;


Logger* logger = Logger::getLogger();


void Chip8::init() {
    opcode = 0;
    I = 0;
    sp = 0;
    delayTimer = 0;
    soundTimer = 0;
    pc = INTERPRETER_SIZE;

    registerAwaitingKeyPress = -1;
    lastProcessorCycleMS = -1;
    lastTimerCycleMS = -1;

    this->clearDisplay();
    this->clearStack();
    this->clearRegisters();
    this->clearKeypad();

    this->copyFontset();

    srand(time(NULL));

    logger->info("Chip8 Initialized!\n");
}

void Chip8::handleKeyDown(int key) {
    this->keypad[key] = 1;
    if (registerAwaitingKeyPress > -1) {
        V[registerAwaitingKeyPress] = key;
        registerAwaitingKeyPress = -1;
        pc += 2;
    }
    logger->debug("handleKeyDown: " + to_string(key) + "\n");
    // logger->display(this->registersToString());
}

void Chip8::handleKeyUp(int key) {
    this->keypad[key] = 0;
    logger->debug("handleKeyUp: " + to_string(key) + "\n");
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

void Chip8::copyFontset() {
    for (int i = 0; i < 80; ++i) {
        memory[i] = chip8Fontset[i];
    }
}

void Chip8::printDisplay() {
    logger->display("printDisplay\n");
    string out = "";
    for (int j = 0; j < DISPLAY_HEIGHT; j ++) {
        for (int i = 0; i < DISPLAY_WIDTH; i++) {
            if (displayBuffer[i + (j * DISPLAY_WIDTH)] == 1) {
                out += 'X';
            } else {
                out += " ";
            }
        }
        out += "\n";
    }

    logger->display(out);
}

string Chip8::registersToString() {
    string out = "";
    for (int i = 0; i < 16; i++) {
        out += to_string(V[i]);
        out += " ";
    }
    out += "\n";
    return out;
}

void Chip8::printStack() {
    logger->display("printStack\n");
    string out = "";
    for (int i = 0; i < 16; i++) {
        out += to_string(stack[i]);
        out += " ";
    }
    out += "\n";

    logger->display(out);
}

string Chip8::keypadToString() {
    string out = "";
    for (int i = 0; i < 16; i++) {
        out += to_string(keypad[i]);
        out += " ";
    }
    out += "\n";
    return out;
}

void Chip8::load(const char *romPath) {
    logger->info("Loading ROM: " + string(romPath) + "\n");

    this->init();

    // Get the file size in bytes
    struct stat fileStat;
    int romFileSize = 0;
    if (stat(romPath, &fileStat) == 0) {
        romFileSize = fileStat.st_size;
        logger->info("ROM File size (bytes): " + to_string(romFileSize) + "\n");
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

    logger->info("ROM loaded into memory!\n");
}

void Chip8::cycle() {
    // Fetch Opcode
    // opcode is two bytes long and located at the program counter
    // shift the first byte by 8 and OR it with the following byte
    opcode = memory[pc] << 8 | memory[pc + 1];
    // cout << "PC: " << pc << endl;
    // cout << endl << "OPCODE: " << hex << opcode;

    long long now = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()).time_since_epoch().count();
    if (lastProcessorCycleMS < 0) {
        lastProcessorCycleMS = now;
    }
    if (lastTimerCycleMS < 0) {
        lastTimerCycleMS = now;
    }
    // cout << lastProcessorCycleMS << " " << now << " " << ((now - lastProcessorCycleMS) * (60 / (float)1000)) << endl;
    bool processOpcode = (now - lastProcessorCycleMS) > 3;
    bool processTimers = (now - lastTimerCycleMS) > 16;

    // Decode Opcode
    if (processOpcode && registerAwaitingKeyPress < 0) {
        lastProcessorCycleMS = now;
        this->handleOpcode();
    }

    // Update timers
    if (processTimers) {
        lastTimerCycleMS = now;

        if (delayTimer) {
            logger->info("Delay timer decrement: " + to_string(delayTimer));
            delayTimer--;
        }
        if (soundTimer) {
            soundTimer--;
        }
    }
}

void Chip8::handleOpcode() {
    switch(opcode & 0xF000) {
        case 0x0000:
            switch(opcode & 0x00FF) {
                case 0x00E0:
                    // 00E0 - CLS
                    // Clear the display.
                    logger->debug(" -- 00E0 Clear display\n");
                    this->clearDisplay();
                    pc += 2;
                    break;
                case 0x00EE:
                    // 00EE - RET
                    // Return from a subroutine.
                    logger->debug(" -- 00EE Return from subroutine\n");
                    pc = stack[--sp];
                    logger->debug("  Removed from stack: " + to_string(pc) + "\n");
                    pc += 2;
                    break;
                // case 0x00C0: (super chip-48)
                    // 00Cn - SCD nibble
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
                    break;
                default:
                    // 0nnn - SYS addr (unnecessary, I think?)
                    cout << "Unhandled 0nnn instruction";
                    throw "Unhandled 0nnn instruction";
            }
            break;
        case 0x1000:
            // 1nnn - JP addr
            // Jump to location nnn.
            pc = opcode & 0x0FFF;
            logger->debug(" -- 1nnn Jump to location: " + to_string(pc) + "\n");
            break;
        case 0x2000:
            // 2nnn - CALL addr
            // Call subroutine at nnn.
            stack[sp++] = pc;
            logger->debug(" -- 2nnn Add to stack: " + to_string(pc) + "\n");
            // this->printStack();
            pc = opcode & 0x0FFF;
            break;
        case 0x3000:
            // 3xkk - SE Vx, byte
            // Skip next instruction if Vx = kk.
            pc += V[(opcode & 0x0F00) >> 8] == (opcode & 0x00FF) ? 4 : 2;
            logger->debug(" -- 3xkk Skip if Vx == kk, pc set to " + to_string(pc) + "\n");
            break;
        case 0x4000:
            // 4xkk - SNE Vx, byte
            // Skip next instruction if Vx != kk.
            pc += V[(opcode & 0x0F00) >> 8] != (opcode & 0x00FF) ? 4 : 2;
            logger->debug(" -- 4xkk Skip if Vx != kk, pc set to " + to_string(pc) + "\n");
            break;
        case 0x5000:
            // 5xy0 - SE Vx, Vy
            // Skip next instruction if Vx = Vy.
            pc += V[(opcode & 0x0F00) >> 8] == V[(opcode & 0x00F0) >> 4] ? 4 : 2;
            logger->debug(" -- 5xy0 Skip if Vx = Vy, pc set to " + to_string(pc) + "\n");
            break;
        case 0x6000:
            // 6xkk - LD Vx, byte
            // Set Vx = kk.
            V[(opcode & 0x0F00) >> 8] = (opcode & 0x00FF);
            logger->debug(" -- 6xkk Set Vx = kk \n");
            logger->display(this->registersToString());
            pc += 2;
            break;
        case 0x7000:
            // 7xkk - ADD Vx, byte
            // Set Vx = Vx + kk.
            logger->debug(" -- 7xkk\n");
            V[(opcode & 0x0F00) >> 8] += (opcode & 0x00FF);
            pc += 2;
            break;
        case 0x8000:
            switch(opcode & 0x000F) {
                case 0x0000:
                    // 8xy0 - LD Vx, Vy
                    // Set Vx = Vy.
                    logger->debug(" -- 8xy0\n");
                    V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4];
                    pc += 2;
                    break;
                case 0x0001:
                    // 8xy1 - OR Vx, Vy
                    // Set Vx = Vx OR Vy.
                    logger->debug(" -- 8xy1\n");
                    V[(opcode & 0x0F00) >> 8] |= V[(opcode & 0x00F0) >> 4];
                    pc += 2;
                    break;
                case 0x0002:
                    // 8xy2 - AND Vx, Vy
                    // Set Vx = Vx AND Vy.
                    logger->debug(" -- 8xy2\n");
                    V[(opcode & 0x0F00) >> 8] &= V[(opcode & 0x00F0) >> 4];
                    pc += 2;
                    break;
                case 0x0003:
                    // 8xy3 - OR Vx, Vy
                    // Set Vx = Vx XOR Vy.
                    logger->debug(" -- 8xy3\n");
                    V[(opcode & 0x0F00) >> 8] ^= V[(opcode & 0x00F0) >> 4];
                    pc += 2;
                    break;
                case 0x0004:
                    // 8xy4 - ADD Vx, Vy
                    // Set Vx = Vx + Vy, set VF = carry.
                    logger->debug(" -- 8xy4\n");
                    V[0xF] = (V[(opcode & 0x0F00) >> 8] + V[(opcode & 0x00F0) >> 4]) > 0xFF ? 1 : 0;
                    V[(opcode & 0x0F00) >> 8] += V[(opcode & 0x00F0) >> 4];
                    pc += 2;
                    break;
                case 0x0005:
                    // 8xy5 - SUB Vx, Vy
                    // Set Vx = Vx - Vy, set VF = NOT borrow.
                    logger->debug(" -- 8xy5\n");
                    // if Vx > Vy, no borrow necessary, VF = 1
                    V[0xF] = V[(opcode & 0x0F00) >> 8] > V[(opcode & 0x00F0) >> 4] ? 1 : 0;
                    V[(opcode & 0x0F00) >> 8] -= V[(opcode & 0x00F0) >> 4];
                    pc += 2;
                    break;
                case 0x0006:
                    // 8xy6 - SHR Vx {, Vy}
                    // Set Vx = Vy SHR 1.
                    logger->debug(" -- 8xy6\n");

                    // If the least-significant bit of Vy is 1, then VF is set to 1, otherwise 0.
                    if (legacyShift) {
                        V[0xF] = V[(opcode & 0x00F0) >> 4] & 0x1;
                        V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4] >> 1;
                    } else {
                        V[0xF] = V[(opcode & 0x0F00) >> 8] & 0x1;
                        V[(opcode & 0x0F00) >> 8] >>= 1;
                    }
                    pc += 2;
                    break;
                case 0x0007:
                    // 8xy7 - SUBN Vy, Vy
                    // Set Vx = Vy - Vx, set VF = NOT borrow.
                    logger->debug(" -- 8xy7\n");

                    // if Vy > Vx, no borrow necessary, VF = 1
                    V[0xF] = V[(opcode & 0x00F0) >> 4] > V[(opcode & 0x0F00) >> 8] ? 1 : 0;
                    V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4] - V[(opcode & 0x0F00) >> 8];
                    pc += 2;
                    break;
                case 0x000E:
                    // 8xyE - SHL Vx {, Vy}
                    // Set Vx = Vy SHL 1.
                    // If the most-significant bit of Vy is 1, then VF is set to 1, otherwise to 0.
                    logger->debug(" -- 8xyE\n");
                    if (legacyShift) {
                        V[0xF] = V[(opcode & 0x00F0) >> 4] >> 7;
                        V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4] << 1;
                    } else {
                        V[0xF] = V[(opcode & 0x0F00) >> 8] >> 7;
                        V[(opcode & 0x0F00) >> 8] <<= 1;
                    }
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
            logger->debug(" -- 9xy0\n");
            pc += V[(opcode & 0x0F00) >> 8] != V[(opcode & 0x00F0) >> 4] ? 4 : 2;
            break;
        case 0xA000:
            // Annn - LD I, addr
            // Set I = nnn.
            logger->debug(" -- Annn\n");
            I = opcode & 0x0FFF;
            pc += 2;
            break;
        case 0xB000:
            // Bnnn - JP V0, addr
            // Jump to location nnn + V0.
            logger->debug(" -- Bnnn\n");
            pc = (opcode & 0x0FFF) + V[0];
            break;
        case 0xC000:
            // Cxkk - RND Vx, byte
            // Set Vx = random byte AND kk.
            logger->debug(" -- Cxkk\n");
            V[(opcode & 0x0F00) >> 8] = (rand() % 256) & (opcode & 0x00FF);
            pc += 2;
            break;
        case 0xD000:
        {
            // Dxyn - DRW Vx, Vy, nibble
            // Display n-byte sprite starting at memory location I at (Vx, Vy), set VF = collision.
            logger->debug(" -- Dxyn\n");
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
            requiresRerender = true;
            pc += 2;

            // this->printDisplay();
            break;
        }
        case 0xE000:
            switch(opcode & 0x00FF) {
                case 0x009E:
                {

                    // Ex9E - SKP Vx
                    // Skip next instruction if key with the value of Vx is pressed.
                    logger->debug(" -- Ex9E\n");
                    // cout << "  Looking for : " << to_string(V[(opcode & 0x0F00) >> 8]) << endl;;
                    logger->debug(this->keypadToString());
                    pc += keypad[V[(opcode & 0x0F00) >> 8]] == 1 ? 4 : 2;
                    break;
                }
                case 0x00A1:
                {

                    // ExA1 - SKNP Vx
                    // Skip next instruction if key with the value of Vx is not pressed.
                    logger->debug(" -- ExA1\n");
                    logger->debug(this->keypadToString());
                    // cout << "  Looking for : " << to_string(V[(opcode & 0x0F00) >> 8]) << endl;;
                    pc += keypad[V[(opcode & 0x0F00) >> 8]] == 0 ? 4 : 2;
                    break;
                }
                default:
                    cout << "Unhandled :( " << hex << opcode << "\n";
                    throw 2;
            }
            break;
        case 0xF000:
            switch(opcode & 0x00FF) {
                case 0x0007:
                    // Fx07 - LD Vx, DT
                    // Set Vx = delay timer value.
                    logger->debug(" -- Fx07\n");
                    V[(opcode & 0x0F00) >> 8] = delayTimer;
                    pc += 2;
                    break;
                case 0x000A:
                    // Fx0A - LD Vx, K
                    // Wait for a key press, store the value of the key in Vx.

                    // When awaiting a press, `registerAwaitingKeyPress` will hold the
                    // register index that needs the press. We'll capture this when
                    // handling the key press in `handleKeyDown`.
                    logger->debug(" -- Fx0A\n");
                    registerAwaitingKeyPress = ((opcode & 0x0F00) >> 8);
                    logger->info("Awaiting key press: " + to_string(registerAwaitingKeyPress) + "\n");
                    break;
                case 0x0015:
                    // Fx15: - LD DT, Vx
                    // Set delay timer = Vx.
                    delayTimer = V[(opcode & 0x0F00) >> 8];
                    logger->debug(" -- Fx15 Set delay timer to " + to_string(delayTimer) + "\n");
                    pc += 2;
                    break;
                case 0x0018:
                    // Fx18 - LD ST, Vx
                    // Set sound timer = Vx.
                    logger->debug(" -- Fx18\n");
                    soundTimer = V[(opcode & 0x0F00) >> 8];
                    pc += 2;
                    break;
                case 0x001E:
                    // Fx1E - ADD I, Vx
                    // Set I = I + Vx.
                    logger->debug(" -- Fx1E\n");
                    I += V[(opcode & 0x0F00) >> 8];
                    pc += 2;
                    break;
                case 0x0029:
                    // Fx29 - LD F, Vx
                    // Set I = location of sprite for digit Vx.
                    // The fontset is loaded as first 80 bytes, each represented
                    // value is 5 bytes long, meaning that "1" is bytes 0-4,
                    // "2" is bytes 5-9 and so on.
                    logger->debug(" -- Fx29\n");
                    I = V[(opcode & 0x0F00) >> 8] * 0x5;
                    pc += 2;
                    break;
                // case 0x0030: (super chip-48)
                    // Fx30 - LD HF, Vx
                case 0x0033:
                    // Fx33 - LD B, Vx
                    // Store BCD representation of Vx in memory locations I, I+1, and I+2.
                    {
                        logger->debug(" -- Fx33\n");
                        unsigned short vx = V[(opcode & 0x0F00) >> 8];
                        memory[I] = vx / 100;
                        memory[I + 1] = (vx / 10) % 10;
                        memory[I + 2] = vx % 10;

                        logger->debug("  VX: " + to_string(vx) + "\n");
                        logger->debug("  Stored BCD: " + to_string(memory[I]) + " " + to_string(memory[I + 1]) + " " + to_string(memory[I + 2]) + "\n");
                        pc += 2;
                        break;
                    }
                case 0x0055:
                    // Fx55 - LD [I], Vx
                    // Store registers V0 through Vx in memory starting at location I.
                    {
                        logger->debug(" -- Fx55\n");
                        unsigned short endX = (opcode & 0x0F00) >> 8;
                        for (int i = 0; i <= endX; i++) {
                            memory[I + i] = V[i];
                        }
                        pc += 2;
                        break;
                    }
                case 0x0065:
                    // Fx65 - LD Vx, [I]
                    // Read registers V0 through Vx from memory starting at location I.
                    {
                        logger->debug(" -- Fx65\n");
                        unsigned short endX = (opcode & 0x0F00) >> 8;
                        for (int i = 0; i <= endX; i++) {
                            V[i] = memory[I + i];
                        }
                        pc += 2;
                        break;
                    }

                // case 0x0075: (super chip-48)
                    // Fx75 - LD R, Vx
                // case 0x85: (super chip-48)
                    // Fx85 - LD Vx, R
                default:
                    cout << "Unhandled " << hex << opcode << "\n";
                    throw "asdf";
            }
            break;
    }
}
