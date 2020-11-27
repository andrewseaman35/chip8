#ifndef CHIP_8_H
#define CHIP_8_H

#include <stdint.h>

class Chip8 {
private:
    uint16_t opcode;

    uint8_t memory[4096]; // "These machines had 4096 (0x1000) memory locations, all of which are 8 bits"

    uint8_t V[16]; // "CHIP-8 has 16 8-bit data registers named V0 to VF."
    uint16_t I; // "The address register, which is named I, is 16 bits wide"

    uint8_t stack[48]; // "The original RCA 1802 version allocated 48 bytes"

    // CHIP-8 has two timers. They both count down at 60 hertz, until they reach 0.
    uint8_t delay_timer;
    uint8_t sound_timer;

    uint16_t pc; // program counter



    void init();

public:
    uint8_t display_buffer[64*32]; // Original CHIP-8 Display resolution is 64×32 pixels
    uint8_t keyboard[16]; // Input is done with a hex keyboard that has 16 keys ranging 0 to F

    void test();

};

#endif // CHIP_8_H
