#ifndef __CHIP8_H__
#define __CHIP8_H__

#include <SDL2/SDL.h>

#define MEMORY_SIZE 4096
#define SCREEN_WIDTH 64
#define SCREEN_HEIGHT 32
#define STACK_SIZE 16

struct Chip8 {
    unsigned short opcode; 				// current OPCODE
	unsigned short I;					// I register for holding addr
	unsigned short pc;					// Program Counter
	unsigned char memory[MEMORY_SIZE];	// Memory, code starting @addr hex 200 == dec 512
	unsigned char V[16];				// Usable registers

	signed char waiting;				// flag asking to wait on a keypress

	unsigned short drawFlag;			// Indicates the screen info has changed and needs to be redrawn
	unsigned char gfx[SCREEN_WIDTH * SCREEN_HEIGHT];         // Stores current screen state

	unsigned char delay_timer;
	unsigned char sound_timer;

	unsigned short stack[STACK_SIZE];	// Call stack stores return addr
	short sp; 							// stack ptr

	unsigned char key[16];				// key states
};

void initialize(struct Chip8* chip);
int loadProgram(struct Chip8* chip, const char* fileName);
void cycle(struct Chip8* chip);

#ifdef DEBUG
void printDebug(struct Chip8* chip);
#endif // DEBUG

#endif // __CHIP8_H__
