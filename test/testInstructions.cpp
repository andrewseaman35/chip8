#include <iostream>

#include "../src/constants.h"
#include "../src/chip8.h"

using namespace std;


class TestChip8: public Chip8 {
private:
    void assertTrue(bool assertion, string err) {
        if (!assertion) {
            cout << "\nAssertionFailed: " << err << "\n";
            throw;
        }
    }
    void test00E0() {
        printf("\n..Testing 00E0\n");
        init();
        for (int i = 0; i < (DISPLAY_WIDTH * DISPLAY_HEIGHT); i++) {
            displayBuffer[i] = 3;
        }

        opcode = 0x00E0;
        handleOpcode();

        for (int i = 0; i < (DISPLAY_WIDTH * DISPLAY_HEIGHT); i++) {
            if (displayBuffer[i] != 0) {
                printf("displayBuffer not cleared at index %u\n", i);
                throw;
            }
        }
    }

    void test00EE() {
        printf("\n..Testing 00EE\n");
        init();
        pc = 151;
        sp = 1;
        stack[0] = 80;

        opcode = 0x00EE;
        handleOpcode();

        assertTrue(pc == 82, "pc != 82");
        assertTrue(sp == 0, "sp != 0");
    }

    void test1nnn() {
        printf("\n..Testing 1nnn\n");
        init();
        pc = 100;

        opcode = 0x1832;
        handleOpcode();

        assertTrue(pc == 0x0832, "pc not updated");
    }

    void test2nnn() {
        printf("\n..Testing 2nnn\n");
        init();
        pc = 100;
        sp = 4;

        opcode = 0x2632;
        handleOpcode();

        assertTrue(stack[4] == 100, "pc not saved to stack");
        assertTrue(sp == 5, "sp not incremented: " + to_string(sp));
        assertTrue(pc == 0x0632, "bad pc");
    }

    void test3xkk() {
        printf("\n..Testing 3xkk");

        // register matches skip byte
        init();
        V[4] = 0x00b1;
        pc = 8;

        opcode = 0x34b1;
        handleOpcode();

        assertTrue(pc == 12, "didn't skip");

        // register does not match skip byte
        init();
        V[5] = 0x0042;
        pc = 12;

        opcode = 0x3541;
        handleOpcode();

        assertTrue(pc == 14, "skipped unnecessarily");
    }

    void test4xkk() {
        printf("\n..Testing 4xkk");

        // register matches skip byte
        init();
        V[4] = 0x00b1;
        pc = 8;

        opcode = 0x44b1;
        handleOpcode();

        assertTrue(pc == 10, "skipped unnecessarily");

        // register does not match skip byte
        init();
        V[5] = 0x0042;
        pc = 12;

        opcode = 0x4541;
        handleOpcode();

        assertTrue(pc == 16, "did not skip");
    }

    void test5xy0() {
        printf("\n..Testing 5xy0\n");

        // Vx == Vy
        init();
        V[3] = 5;
        V[7] = 5;
        pc = 2;
        opcode = 0x5370;
        handleOpcode();

        assertTrue(pc == 6, "didn't skip");

        // Vx != Vy
        init();
        V[3] = 6;
        V[7] = 9;
        pc = 8;
        opcode = 0x5730;
        handleOpcode();

        assertTrue(pc == 10, "skipped unnecessarily");
    }

    void test6xkk() {
        printf("\n..Testing 6xkk\n");

        init();
        V[8] = 9;
        pc = 8;
        opcode = 0x6833;
        handleOpcode();

        assertTrue(V[8] == 0x0033, "register not updated");
        assertTrue(pc == 10, "bad pc");
    }

    void test7xkk() {
        printf("\n..Testing 7xkk\n");

        init();
        V[8] = 9;
        pc = 8;
        opcode = 0x7832;
        handleOpcode();

        // int(0x32) = 50
        assertTrue(V[8] == 59, "register not set properly");
        assertTrue(pc == 10, "bad pc");
    }

    void test8xy0() {
        printf("\n..Testing 8xy0\n");

        init();
        V[8] = 9;
        V[12] = 3;
        pc = 8;
        opcode = 0x88C0;
        handleOpcode();

        assertTrue(V[8] == 3, "register not set");
        assertTrue(V[8] == V[12], "registers not samesies");
        assertTrue(pc == 10, "bad pc");
    }

    void test8xy1() {
        printf("\n..Testing 8xy1\n");

        init();
        V[8] = 9;
        V[12] = 3;
        pc = 8;
        opcode = 0x88C1;
        handleOpcode();

        assertTrue(V[8] == 11, "register not set");
        assertTrue(pc == 10, "bad pc");
    }

    void test8xy2() {
        printf("\n..Testing 8xy2\n");

        init();
        V[8] = 9;
        V[12] = 3;
        pc = 8;
        opcode = 0x88C2;
        handleOpcode();

        assertTrue(V[8] == 1, "register not set: " + to_string(V[8]));
        assertTrue(pc == 10, "bad pc");
    }

    void test8xy3() {
        printf("\n..Testing 8xy3\n");

        init();
        V[8] = 9;
        V[12] = 3;
        pc = 8;
        opcode = 0x88C3;
        handleOpcode();

        assertTrue(V[8] == 10, "register not set: " + to_string(V[8]));
        assertTrue(pc == 10, "bad pc");
    }

    void test8xy4() {
        printf("\n..Testing 8xy4\n");

        // with carry
        init();
        V[8] = 200;
        V[0xC] = 88;
        pc = 8;
        opcode = 0x88C4;
        handleOpcode();

        assertTrue(V[8] == 32, "register not set: " + to_string(V[8]));
        assertTrue(V[0xF] == 1, "VF register not set: " + to_string(V[0xF]));
        assertTrue(pc == 10, "bad pc");

        // without carry
        init();
        V[8] = 200;
        V[0xC] = 54;
        pc = 8;
        opcode = 0x88C4;
        handleOpcode();

        assertTrue(V[8] == 254, "register not set: " + to_string(V[8]));
        assertTrue(V[0xF] == 0, "VF register set unnecessarily: " + to_string(V[0xF]));
        assertTrue(pc == 10, "bad pc");
    }

    void test8xy5() {
        printf("\n..Testing 8xy5\n");

        // without carry
        init();
        V[8] = 200;
        V[0xC] = 54;
        pc = 8;
        opcode = 0x88C5;
        handleOpcode();

        printf("testing without borrow\n");
        assertTrue(V[8] == 146, "register not set: " + to_string(V[8]));
        assertTrue(V[0xF] == 1, "VF register set unnecessarily: " + to_string(V[0xF]));
        assertTrue(pc == 10, "bad pc");

        // with carry
        init();
        V[8] = 0x55;
        V[0xC] = 0x88;
        pc = 8;
        opcode = 0x88C5;
        handleOpcode();

        printf("testing with borrow\n");
        assertTrue(V[8] == 205, "register not set: " + to_string(V[8]));
        assertTrue(V[0xF] == 0, "VF no good :(: " + to_string(V[0xF]));
        assertTrue(pc == 10, "bad pc");
    }

    void test8xy6() {
        printf("\n..Testing 8xy6\n");

        // lsb == 1
        init();
        V[0xF] = 9;
        V[4] = 9;
        pc = 12;
        opcode = 0x8416;
        handleOpcode();

        printf("testing lsb = 1\n");
        assertTrue(V[4] == 4, "register not set: " + to_string(V[4]));
        assertTrue(V[0xF] == 1, "VF not set to 1");
        assertTrue(pc == 14, "bad pc");

        // lsb != 1
        init();
        V[0xF] = 9;
        V[4] = 0xA;
        pc = 12;
        opcode = 0x8416;
        handleOpcode();

        printf("test lsb != 0\n");
        assertTrue(V[4] == 5, "register not set: " + to_string(V[4]));
        assertTrue(V[0xF] == 0, "VF not set to 0");
        assertTrue(pc == 14, "bad pc");
    }

    void test8xy7() {
        printf("\n..Testing 8xy7\n");

        // without carry
        init();
        V[8] = 200;
        V[0xC] = 54;
        pc = 8;
        opcode = 0x8C87;
        handleOpcode();

        printf("testing without borrow\n");
        assertTrue(V[0xC] == 146, "register not set: " + to_string(V[0xC]));
        assertTrue(V[0xF] == 1, "VF register set unnecessarily: " + to_string(V[0xF]));
        assertTrue(pc == 10, "bad pc");

        // with carry
        init();
        V[8] = 0x55;
        V[0xC] = 0x88;
        pc = 8;
        opcode = 0x8C87;
        handleOpcode();

        printf("testing with borrow\n");
        assertTrue(V[0xC] == 205, "register not set: " + to_string(V[0xC]));
        assertTrue(V[0xF] == 0, "VF no good :(: " + to_string(V[0xF]));
        assertTrue(pc == 10, "bad pc");
    }

    void test8xyE() {
        printf("\n..Testing 8xyE\n");

        init();
        V[8] = 200;
        V[0xC] = 54;
        pc = 8;
        opcode = 0x8C8E;
        handleOpcode();

        // msb = 0
        printf("testing msb = 0\n");
        assertTrue(V[0xC] == 108, "register not set: " + to_string(V[0xC]));
        assertTrue(V[0xF] == 0, "VF set unnecessarily");
        assertTrue(pc == 10, "bad pc");

        init();
        V[8] = 200;
        V[0xC] = 200;
        pc = 8;
        opcode = 0x8C8E;
        handleOpcode();

        // msb = 0
        printf("testing msb = 0\n");
        assertTrue(V[0xC] == 144, "register not set: " + to_string(V[0xC]));
        assertTrue(V[0xF] == 1, "VF not set");
        assertTrue(pc == 10, "bad pc");
    }

    void test9xy0() {
        printf("\n..Testing 9xy0\n");

        init();
        pc = 8;
        V[4] = 5;
        V[7] = 5;
        opcode = 0x9470;
        handleOpcode();

        printf("testing Vx = Vy\n");
        assertTrue(pc == 10, "bad pc");

        init();
        pc = 8;
        V[4] = 5;
        V[7] = 6;
        opcode = 0x9470;
        handleOpcode();

        printf("testing Vx != Vy\n");
        assertTrue(pc == 12, "bad pc");
    }

    void testAnnn() {
        I = 8;
        opcode = 0xA732;
        pc = 10;
        handleOpcode();

        assertTrue(I == 0x0732, "Bad I: " + to_string(I));
        assertTrue(pc == 12, "bad pc");
    }

    void testBnnn() {
        V[0] = 7;
        pc = 8;
        opcode = 0xB812;
        handleOpcode();

        assertTrue(pc == 0x0819, "bad pc: " + to_string(pc));
    }

public:
    void run() {
        test00E0();
        test00EE();
        test1nnn();
        test2nnn();
        test3xkk();
        test4xkk();
        test5xy0();
        test6xkk();
        test7xkk();
        test8xy0();
        test8xy1();
        test8xy2();
        test8xy3();
        test8xy4();
        test8xy5();
        test8xy6();
        test8xy7();
        test8xyE();
        test9xy0();
        testAnnn();
        testBnnn();
    }
};




int main() {
    TestChip8 chip8 = TestChip8();
    chip8.run();
}
