#ifndef CHIP_8_H
#define CHIP_8_H

#include <stdint.h>

class Chip8 {
private:
    uint16_t opcode;

    uint8_t memory[4096]; // "The Chip-8 language is capable of accessing up to 4KB (4,096 bytes) of RAM"

    uint8_t V[16]; // "Chip-8 has 16 general purpose 8-bit registers"
    uint16_t I; // "There is also a 16-bit register called I"

    uint16_t stack[16]; // "The stack is an array of 16 16-bit values"
    uint8_t sp; // "The stack pointer (SP) can be 8-bit"

    // "Chip-8 also has two special purpose 8-bit registers".
    uint8_t delayTimer;
    uint8_t soundTimer;

    uint16_t pc; // "The program counter (PC) should be 16-bit"

    void clearDisplay();
    void clearStack();
    void clearRegisters();
    void clearKeypad();

    void printRegisters();

public:
    uint8_t displayBuffer[64*32]; // "64x32-pixel monochrome display"
    uint8_t keypad[16]; // "16-key hexadecimal keypad"

    void test();
    void init();
    void load(const char *romPath);
    void cycle();
};

#endif // CHIP_8_H
