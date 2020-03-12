#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "chip8.h"

void initialize(struct Chip8* chip)
{
    unsigned char chip8_fontset[80] = {
        0xF0, 0x90, 0x90, 0x90, 0xF0, 	// 0
        0x20, 0x60, 0x20, 0x20, 0x70, 	// 1
        0xF0, 0x10, 0xF0, 0x80, 0xF0, 	// 2
        0xF0, 0x10, 0xF0, 0x10, 0xF0, 	// 3
        0x90, 0x90, 0xF0, 0x10, 0x10, 	// 4
        0xF0, 0x80, 0xF0, 0x10, 0xF0, 	// 5
        0xF0, 0x80, 0xF0, 0x90, 0xF0, 	// 6
        0xF0, 0x10, 0x20, 0x40, 0x40, 	// 7
        0xF0, 0x90, 0xF0, 0x90, 0xF0, 	// 8
        0xF0, 0x90, 0xF0, 0x10, 0xF0, 	// 9
        0xF0, 0x90, 0xF0, 0x90, 0x90, 	// A
        0xE0, 0x90, 0xE0, 0x90, 0xE0, 	// B
        0xF0, 0x80, 0x80, 0x80, 0xF0, 	// C
        0xE0, 0x90, 0x90, 0x90, 0xE0, 	// D
        0xF0, 0x80, 0xF0, 0x80, 0xF0, 	// E
        0xF0, 0x80, 0xF0, 0x80, 0x80, 	// F
    }; // bitwise encoded sprites for the built-in fontset

    chip->opcode = 0;
    chip->I = 0;
    chip->pc = 0x200;
    chip->drawFlag = 0;
    chip->sp = 0;
    chip->delay_timer = 0;
    chip->sound_timer = 0;

    memset(chip->gfx, 0, SCREEN_HEIGHT * SCREEN_WIDTH * sizeof(char));
    memset(chip->stack, 0, 16 * sizeof(short));
    memset(chip->V, 0, 16 * sizeof(char));
    memset(chip->memory, 0, MEMORY_SIZE * sizeof(char));

    memcpy(chip->memory, chip8_fontset, 80 * sizeof(char));

    srand(time(NULL));
}

int loadProgram(struct Chip8* chip, const char* fileName)
{
    FILE* program = fopen(fileName, "rb");
    if (!program) {
        perror("Error ");
        return -1;
    }

    // find the file size
    fseek(program, 0, SEEK_END);
    long size = ftell(program);
    rewind(program);

    if (size > 4096 - 0x200) {
        printf("Error: %s is larger than a Chip8 program\n", fileName);
        return -1;
    }

    long result = fread(chip->memory + 0x200, sizeof(char), size, program);
    if (result != size) {
        printf("Error: Problem reading file\n");
        return -1;
    }

    fclose(program);

    /// Create an SDL Environment to run in:
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL Initialization failure\n");
        return -1;
    }
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");

    int width = SCREEN_WIDTH * 8;
    int height = SCREEN_HEIGHT * 8;

    chip->window = SDL_CreateWindow(fileName, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_SHOWN);
    if (!chip->window) {
        printf("SDL Window creation failure\n");
        SDL_Quit();
        return -1;
    }

    chip->renderer = SDL_CreateRenderer(chip->window, -1, SDL_RENDERER_ACCELERATED);
    if (!chip->renderer) {
        printf("SDL Renderer creation failure\n");
        SDL_DestroyWindow(chip->window);
        SDL_Quit();
        return -1;
    }

    return 0;
}

void op0(struct Chip8* chip);
void op1(struct Chip8* chip);
void op2(struct Chip8* chip);
void op3(struct Chip8* chip);
void op4(struct Chip8* chip);
void op5(struct Chip8* chip);
void op6(struct Chip8* chip);
void op7(struct Chip8* chip);
void op8(struct Chip8* chip);
void op9(struct Chip8* chip);
void opA(struct Chip8* chip);
void opB(struct Chip8* chip);
void opC(struct Chip8* chip);
void opD(struct Chip8* chip);
void opE(struct Chip8* chip);
void opF(struct Chip8* chip);

void cycle(struct Chip8* chip)
{
    /// The opcode function table. Static and unchanging.
    static void (*chip8Table[16]) (struct Chip8* chip) = {
        op0, op1, op2, op3,
        op4, op5, op6, op7,
        op8, op9, opA, opB,
        opC, opD, opE, opF
    };

    /// for the command to wait on a keypress without looping forever in a command
    /// (and causing the event handler to thus be paused), we store that we're waiting
    /// for a keypress and hijack the cycle here, before reading the next opcode.
    /// If no key is pressed, we abort here.
    if (chip->waiting) {
        for (int i = 0; i < 16; ++i) {
            if (chip->key[i]) {
                chip->V[(chip->opcode & 0x0F00) >> 8] = i;
                chip->waiting = 0;
                goto endif;
            }
        }
        return;
    } endif:;

    /// read the next opcode and increment the PC
    chip->opcode = chip->memory[chip->pc] << 8 | chip->memory[chip->pc + 1];
    chip->pc += 2;

    /// Dispatch
    chip8Table[(chip->opcode & 0xF000) >> 12](chip);
}

/// opcode | effect
/// ----------------------------------------------
/// 0x0NNN | Call RCA 1802 program at address NNN  --- Not implemented. Not neccessary for most modern applications
/// 0x00E0 | Clear the screen
/// 0x00EE | jump to the last address in the stack
void op0(struct Chip8* chip)
{
    if (chip->opcode == 0x00E0) {
        memset(chip->gfx, 0, SCREEN_HEIGHT * SCREEN_WIDTH * sizeof(char));
        chip->drawFlag = 1;
    } else if (chip->opcode == 0x00EE) {
        chip->pc = chip->stack[chip->sp];
        chip->stack[chip->sp] = 0;
        if (--chip->sp < 0) {
            chip->sp = 0;
        }
    }
}

/// opcode | effect
/// ----------------------------------------------
/// 0x1NNN | jump to address NNN
void op1(struct Chip8* chip)
{
    chip->pc = chip->opcode & 0x0FFF;
}

/// opcode | effect
/// ----------------------------------------------
/// 0x2NNN | Call subroutine at address NNN
void op2(struct Chip8* chip)
{
    chip->sp++;
    chip->stack[chip->sp] = chip->pc;
    chip->pc = chip->opcode & 0x0FFF;
}

/// opcode | effect
/// ----------------------------------------------
/// 0x3XNN | Skip next instruction if VX == NN
void op3(struct Chip8* chip)
{
    if (chip->V[(chip->opcode & 0x0F00) >> 8] == (chip->opcode & 0x00FF)) {
        chip->pc += 2;
    }
}

/// opcode | effect
/// ----------------------------------------------
/// 0x4XNN | Skip next instruction if VX != NN
void op4(struct Chip8* chip)
{
    if (chip->V[(chip->opcode & 0x0F00) >> 8] != (chip->opcode & 0x00FF)) {
        chip->pc += 2;
    }
}

/// opcode | effect
/// ----------------------------------------------
/// 0x4XY0 | Skip next instruction if VX == VY
void op5(struct Chip8* chip)
{
    if (chip->V[(chip->opcode & 0x0F00) >> 8] == chip->V[(chip->opcode & 0x00F0) >> 4]) {
        chip->pc += 2;
    }
}

/// opcode | effect
/// ----------------------------------------------
/// 0x6XNN | Set VX = NN
void op6(struct Chip8* chip)
{
    chip->V[(chip->opcode & 0x0F00) >> 8] = chip->opcode & 0x00FF;
}

/// opcode | effect
/// ----------------------------------------------
/// 0x7XNN | Set VX += NN
void op7(struct Chip8* chip)
{
    chip->V[(chip->opcode & 0x0F00) >> 8] += chip->opcode & 0x00FF;
}

/// opcode | effect
/// ----------------------------------------------
/// 0x8XY0 | Set VX = VY
/// 0x8XY1 | Set VX |= VY  (VX = VX OR  VY)
/// 0x8XY2 | Set VX &= VY  (VX = VX AND VY)
/// 0x8XY3 | Set VX ^= VY  (VX = VX XOR VY)
/// 0x8XY4 | Set VX += VY, VF Set to 1 if there is a carry, 0 if not
/// 0x8XY5 | Set VX -= VY, VF set to 0 if there is a borrow, 1 if not
/// 0x8XY6 | Store the least significant bit of VX in VF, then VX >>= 1
/// 0x8XY7 | Set VX = VY - VX, VF set to 0 if there is a borrow, 1 if not
/// 0x8XYE | Store the most significant bif of VX in VF, then VX <<= 1;
void op8(struct Chip8* chip)
{
    // decide which opcode to execute.
    unsigned char R1 = (chip->opcode & 0x0F00) >> 8;
    unsigned char R2 = (chip->opcode & 0x00F0) >> 4;
    short result;
    switch (chip->opcode & 0x000F) {
    case 0:
        chip->V[R1] = chip->V[R2];
        break;
    case 1:
        chip->V[R1] |= chip->V[R2];
        break;
    case 2:
        chip->V[R1] &= chip->V[R2];
        break;
    case 3:
        chip->V[R1] ^= chip->V[R2];
        break;
    case 4:
        chip->V[15] = (chip->V[R1] + chip->V[R2]) >> 8;
        chip->V[R1] += chip->V[R2];
        break;
    case 5:
        result = ( (((short) chip->V[R1]) | 0x100) - chip->V[R2] );
        chip->V[15] = result >> 8;
        chip->V[R1] = result & 0xFF;
        break;
    case 6:
        chip->V[15] = chip->V[R1] & 0x0001;
        chip->V[R1] >>= 1;
        break;
    case 7:
        result = ( (((short) chip->V[R2]) | 0x100) - chip->V[R1] );
        chip->V[15] = result >> 8;
        chip->V[R1] = result & 0xFF;
        break;
    case 0xE:
        chip->V[15] = (chip->V[R1] & 0x80) >> 7;
        chip->V[R1] <<= 1;
        break;
    }
}

/// opcode | effect
/// ----------------------------------------------
/// 0x9XY0 | Skip next intruction if VX != VY
void op9(struct Chip8* chip)
{
    if (chip->V[(chip->opcode & 0x0F00) >> 8] != chip->V[(chip->opcode & 0x00F0) >> 4]) {
        chip->pc += 2;
    }
}

/// opcode | effect
/// ----------------------------------------------
/// 0xANNN | Set I = NNN (NNN an address)
void opA(struct Chip8* chip)
{
    chip->I = 0x0FFF & chip->opcode;
}

/// opcode | effect
/// ----------------------------------------------
/// 0xBNNN | Jump to address NNN + V0
void opB(struct Chip8* chip)
{
    chip->pc = (chip->opcode & 0x0FFF) + chip->V[0];
}

/// opcode | effect
/// ----------------------------------------------
/// 0xCXNN | set VX to a random number ANDed with NN
void opC(struct Chip8* chip)
{
    chip->V[(chip->opcode & 0x0F00) >> 8] = rand() & (chip->opcode & 0x00FF);
}

/// opcode | effect
/// ----------------------------------------------
/// 0xDXYN | Draw a sprite at coordinate (VX, VY) with width 8, height N.
///        | Each row of 8 pixels is read as bit-coded starting from memory
///        | location I. I is unchanged. VF is set to 1 if any screen pixels
///        | are flipped to unset when the sprite is drawn, and 0 if not.
void opD(struct Chip8* chip)
{
    int X = chip->V[(chip->opcode & 0x0F00) >> 8];
    int Y = chip->V[(chip->opcode & 0x00F0) >> 4];
    short height = chip->opcode & 0x000F;
    chip->V[0xF] = 0;
    chip->drawFlag = 1;
    for (int spriteY = 0 ; spriteY < height; ++spriteY) {
        for (int spriteX = 0 ; spriteX < 8; ++spriteX) {
            // calculate the absolute position
            int pos = (Y + spriteY) * SCREEN_WIDTH + X + spriteX;
            if (chip->gfx[pos] && ((chip->memory[chip->I + spriteY] >> (7 - spriteX)) &0x0001)) {
                chip->V[0xF] = 1;
                chip->gfx[pos] = 0;
            } else if (chip->gfx[pos] || ((chip->memory[chip->I + spriteY] >> (7 - spriteX)) &0x0001)) {
                chip->gfx[pos] = 1;
            }
        }
    }
}

/// opcode | effect
/// ----------------------------------------------
/// 0xEX9E | skip next instruction if the key in VX is pressed
/// 0xEXA1 | skip next instruction if the key in VX is not pressed
void opE(struct Chip8* chip)
{
    if ((chip->opcode & 0x00FF) == 0x9E) {
        if (chip->key[(chip->opcode & 0x0F00) >> 8]) {
            chip->pc += 2;
        }
    } else if ((chip->opcode & 0x00FF) == 0xA1) {
        if ( !(chip->key[(chip->opcode & 0x0F00) >> 8]) ) {
            chip->pc += 2;
        }
    }
}

/// opcode | effect
/// ----------------------------------------------
/// 0xFX07 | Set VX to the value of the delay timer
/// 0xFX0A | Wait until a key is pressed, and then stored in VX
/// 0xFX15 | Set the delay timer to VX
/// 0xFX18 | Set the sound timer to VX
/// 0xFX1E | Add VX to I. VF is set to 1 if there is a range overflow, 0 otherwise
/// 0xFX29 | Set I to the location of the sprite for the character in VX
/// 0xFX33 | Stores the binary-encoded decimal representation of VX in addr I
/// 0xFX55 | Stores V0 to VX in memory, starting at addr I. I is left unmodified
/// 0xFX65 | Loads values from memory into V0 - Vx, from addr I. I left unmodified
void opF(struct Chip8* chip)
{
    short X = (chip->opcode & 0x0F00) >> 8;
    switch (chip->opcode & 0x00FF) {
    case 0x07:
        chip->V[X] = chip->delay_timer;
        break;
    case 0x0A:
        // set "waiting" flag.
        chip->waiting = 1;
        break;
    case 0x15:
        chip->delay_timer = chip->V[X];
        break;
    case 0x18:
        chip->sound_timer = chip->V[X];
        break;
    case 0x1E:
        chip->I = chip->I + chip->V[X];
        chip->V[0xF] = (chip->I > 0xFFF);
        chip->I &= 0xFFF;
        break;
    case 0x29:
        chip->I = 5 * chip->V[X];
        break;
    case 0x33:
        chip->memory[chip->I] = chip->V[X] / 100;
        chip->memory[chip->I + 1] = (chip->V[X] / 10) % 10;
        chip->memory[chip->I + 2] = chip->V[X] % 10;
        break;
    case 0x55:
        for (int i = 0 ; i <= ((chip->opcode & 0x0F00) >> 8) ; ++i) {
            chip->memory[chip->I + i] = chip->V[i];
        }
        break;
    case 0x65:
        for (int i = 0 ; i <= ((chip->opcode & 0x0F00) >> 8) ; ++i) {
            chip->V[i] = chip->memory[chip->I + i];
        }
        break;
    }
}
