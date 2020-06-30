This is a project I created to learn the basics of writing emulators.
It is a CHIP-8 Emulator written in C

It uses the SDL2 library for graphics


chip8.h defines the
'''
struct Chip8
'''
structure and the functions needed to load and run chip8 programs
as well as several constant macros which could be useful to know:
'''
MEMORY_SIZE
SCREEN_WIDTH
SCREEN_HEIGHT
STACK_SIZE
'''
'''
void initialize( struct Chip8* )
''' 
Freshly initializes the Chip8 data structure with default values
(Screen is zeroed, PC set to 0x200, font data placed at 0x80)

'''
int loadProgram( struct Chip8*, char* path )
'''
Loads a chip8 program from a file into memory or returns an error code

'''
void cycle( struct Chip8* )
'''
Runs one emulated fetch-decode-execute cycle

## usage:

Use the Makefile provided or compile with your favourite C compiler and link the SDL2 library

'''
"Executable" "Chip-8 program"
'''