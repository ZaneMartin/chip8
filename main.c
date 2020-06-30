#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>

#include "chip8.h"

const int PIXEL_WIDTH = 10;
const int PIXEL_HEIGHT = 10;
const int DEFAULT_CYCLES_PER_SECOND = 500;

int main(int argc, char* argv[])
{
    int cycles_per_second = DEFAULT_CYCLES_PER_SECOND;
	// Convert from human-readable cycles/sec (~analagous to FPS) into ticks/cycle
	// Where 1 tick = 1/1000 Sec, whichh is what the clock counts
    int ticks_per_cycle = 1000 / cycles_per_second;
	
	SDL_Window* window = NULL;
	SDL_Renderer* renderer = NULL;

    struct Chip8 my_chip;
    initialize(&my_chip);

	// Parse arguements
	switch ( argc ) {
		case 2:
			if ( loadProgram(&my_chip, argv[1]) == -1 ){
				fprintf(stderr, "Error loading program \"%s\"\n", argv[1]);
				return -1;
			}
			break;
		case 1:
			printf("Enter the ROM file: \n");
			char path[50];
			scanf("%s", path);
			if (loadProgram(&my_chip, path) == -1) {
				return -1;
			}
			break;
		default:
			printf( "Usage: %s [path]\n", argv[0] );
			return 0;
	}

	// Setup graphics
	if ( SDL_Init( SDL_INIT_VIDEO) < 0 ) {
		goto SDL_CLEANUP_0;
	}

	window = SDL_CreateWindow( "CHIP-8", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 
		SCREEN_WIDTH * PIXEL_WIDTH, SCREEN_HEIGHT * PIXEL_HEIGHT, SDL_WINDOW_SHOWN );
	if ( !window ) {
		goto SDL_CLEANUP_1;
	}

	// NO VSYNC because this uses other timers.
	renderer = SDL_CreateRenderer( window, -1, SDL_RENDERER_ACCELERATED );
	if ( !renderer ) { 
		goto SDL_CLEANUP_2;
	}

    printf("Success! Press + and - to adjust emulation speed\nCurrent Speed is %d instructions per second\n", cycles_per_second);

    Uint32 timer60Hz = SDL_GetTicks();
	
	// Loop until SDL_Quit event or program end
    for(SDL_Event event;;) {
        Uint32 cycleStartTime = SDL_GetTicks();
		// Clear the event queue. Handle quit events as well as change emulation speed requests.
        while(SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                goto breakMainLoop;
            } else if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                case SDLK_EQUALS:
                    cycles_per_second += 10;
                    ticks_per_cycle = 1000 / cycles_per_second;
                    printf("Current Speed is %d instructions per second\n", cycles_per_second);
                    break;
                case SDLK_MINUS:
					if ( cycles_per_second > 10 ) {
                    	cycles_per_second -= 10;
					}
                    ticks_per_cycle = 1000 / cycles_per_second;
                    printf("Current Speed is %d instructions per second\n", cycles_per_second);
                    break;
                }
            }
        }
        /// Populate the keystates.
        /// MAPPING:
        /// 1 | 2 | 3 | 4   |   0 | 1 | 2 | 3
        /// Q | W | E | R   |   4 | 5 | 6 | 7
        /// A | S | D | F   |   8 | 9 | A | B
        /// Z | X | C | V   |   C | D | E | F

        const unsigned char* keys = SDL_GetKeyboardState(NULL);
        my_chip.key[0] = keys[SDL_SCANCODE_X];
        my_chip.key[1] = keys[SDL_SCANCODE_1];
        my_chip.key[2] = keys[SDL_SCANCODE_2];
        my_chip.key[3] = keys[SDL_SCANCODE_3];
        my_chip.key[4] = keys[SDL_SCANCODE_Q];
        my_chip.key[5] = keys[SDL_SCANCODE_W];
        my_chip.key[6] = keys[SDL_SCANCODE_E];
        my_chip.key[7] = keys[SDL_SCANCODE_A];
        my_chip.key[8] = keys[SDL_SCANCODE_S];
        my_chip.key[9] = keys[SDL_SCANCODE_D];
        my_chip.key[10] = keys[SDL_SCANCODE_Z];
        my_chip.key[11] = keys[SDL_SCANCODE_C];
        my_chip.key[12] = keys[SDL_SCANCODE_4];
        my_chip.key[13] = keys[SDL_SCANCODE_R];
        my_chip.key[14] = keys[SDL_SCANCODE_F];
        my_chip.key[15] = keys[SDL_SCANCODE_V];

        // operate next instruction.
        cycle(&my_chip);

        // Redraw the screen if anything has changed
        if (my_chip.drawFlag) {
			// Create a pixel rectangle at (0, 0)
            SDL_Rect pixel = { 0, 0, PIXEL_WIDTH, PIXEL_HEIGHT };
			// Clear screen
            SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);
            SDL_RenderClear(renderer);
			// Set draw colour to whatever we want pixels to be. I use white
            SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
            for ( int y = 0 ; y < SCREEN_HEIGHT ; ++y ) {
                for ( int x = 0 ; x < SCREEN_WIDTH ; ++x ) {
                    if (my_chip.gfx[y * SCREEN_WIDTH + x]) { // If there is an active pixel here
						// Move the pixel rectangle to the correct position on the screen and render it
                        pixel.x = x * PIXEL_WIDTH;
                        pixel.y = y * PIXEL_HEIGHT;
                        SDL_RenderFillRect(renderer, &pixel);
                    }
                }
            }

			// Draw the screen and reset the drawFlag
            SDL_RenderPresent(renderer);
            my_chip.drawFlag = 0;
        }
		// Check if 1/60th of a second has elapsed
        while (SDL_GetTicks() - timer60Hz > 1000 / 60) {
			// If it has, increment the 60Hz timer and decrement the delay and sound timers.
            timer60Hz += 1000 / 60;
            if (my_chip.delay_timer) {
                my_chip.delay_timer--;
            }
            if (my_chip.sound_timer) {
				// Beep on non-zero
				printf("\7");
                my_chip.sound_timer--;
            }
        }
        if (SDL_GetTicks() - cycleStartTime < ticks_per_cycle) {
            SDL_Delay(ticks_per_cycle - (SDL_GetTicks() - cycleStartTime));
        }
    } breakMainLoop:


    SDL_DestroyRenderer( renderer );
SDL_CLEANUP_2:
    SDL_DestroyWindow( window );
SDL_CLEANUP_1:
    SDL_Quit();
SDL_CLEANUP_0:

    return 0;
}
