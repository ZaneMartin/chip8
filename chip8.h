#ifndef __CHIP8_H__
#define __CHIP8_H__

#include <SDL2/SDL.h>

#define MEMORY_SIZE 4096
#define SCREEN_WIDTH 64
#define SCREEN_HEIGHT 32
#define STACK_SIZE 16

struct Chip8 {

    //
    unsigned short opcode;
	unsigned short I;
	unsigned short pc;
	unsigned char memory[MEMORY_SIZE];
	unsigned char V[16];

	signed char waiting;

	unsigned short drawFlag;
	unsigned char gfx[SCREEN_WIDTH * SCREEN_HEIGHT];         // Stores current screen state

	unsigned char delay_timer;
	unsigned char sound_timer;

	unsigned short stack[STACK_SIZE];
	short sp; //stack ptr

	unsigned char key[16];

	SDL_Window* window;
	SDL_Renderer* renderer;
};

void initialize(struct Chip8* chip);
int loadProgram(struct Chip8* chip, const char* fileName);
void cycle(struct Chip8* chip);

#endif // __CHIP8_H__
