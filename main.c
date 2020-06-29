#include <stdio.h>
#include <stdlib.h>

#include "chip8.h"

const int DEFAULT_CYCLES_PER_SECOND = 500;

int main(int argc, char* argv[])
{
    int cycles_per_second = DEFAULT_CYCLES_PER_SECOND;
    int ticks_per_cycle = 1000 / cycles_per_second;

    struct Chip8 my_chip;

    initialize(&my_chip);

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
		default:
			printf( "Usage: %s [path]\n", argv[0] );
			return 0;
	}

    printf("Success! Press + and - to adjust emulation speed\nCurrent Speed is 500 instructions per second\n");

    Uint32 timer60Hz = SDL_GetTicks();
    for(SDL_Event event;;) {
        Uint32 cycleStartTime = SDL_GetTicks();
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

        // render
        if (my_chip.drawFlag) {
            SDL_Rect pixel = { 0, 0, 8, 8 };
            SDL_SetRenderDrawColor(my_chip.renderer, 0x00, 0x00, 0x00, 0xFF);
            SDL_RenderClear(my_chip.renderer);
            SDL_SetRenderDrawColor(my_chip.renderer, 0xFF, 0xFF, 0xFF, 0xFF);
            for (int y = 0 ; y < SCREEN_HEIGHT ; ++y) {
                for(int x = 0 ; x < SCREEN_WIDTH ; ++x) {
                    if (my_chip.gfx[y * SCREEN_WIDTH + x]) {
                        pixel.x = x * 8;
                        pixel.y = y * 8;
                        SDL_RenderFillRect(my_chip.renderer, &pixel);
                    }
                }
            }
            SDL_RenderPresent(my_chip.renderer);
            my_chip.drawFlag = 0;
        }
        if (SDL_GetTicks() - timer60Hz > 1000 / 60) {
            timer60Hz += 1000 / 60;
            if (my_chip.delay_timer) {
                my_chip.delay_timer--;
            }
            if (my_chip.sound_timer) {
                my_chip.sound_timer--;
            }
        }
        if (SDL_GetTicks() - cycleStartTime < ticks_per_cycle) {
            SDL_Delay(ticks_per_cycle - (SDL_GetTicks() - cycleStartTime));
        }

        #ifdef DEBUG
        printDebug(&my_chip);
        for(;;) {
            if (SDL_PollEvent(&event)) {
                if (event.type == SDL_QUIT) {
                    goto breakMainLoop;
                } else if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_RETURN) {
                    break;
                }
            }
        }
        #endif // DEBUG

    } breakMainLoop:;

    SDL_DestroyRenderer(my_chip.renderer);
    SDL_DestroyWindow(my_chip.window);
    SDL_Quit();

    return 0;
}
