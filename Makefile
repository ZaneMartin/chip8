srcFiles = $(wildcard *.c)
objFiles = $(srcFiles:.c=.o)

CC = gcc

COMPILER_FLAGS = -Wall -std=c11

LINKER_FLAGS = -lSDL2 -lSDL2_image -lSDL2_ttf

# target
all : $(objFiles) 
	$(CC) $^ $(COMPILER_FLAGS) $(LINKER_FLAGS)

.PHONY: clean
clean:
	rm -f $(objFiles) a.out

