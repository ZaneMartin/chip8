srcFiles = $(wildcard *.c)
objFiles = $(srcFiles:.c=.o)

CC = gcc

COMPILER_FLAGS = -Wall -std=c11

LINKER_FLAGS = -lSDL2 

# target
all : $(objFiles) 
	$(CC) $^ $(COMPILER_FLAGS) $(LINKER_FLAGS)

.PHONY: debug
debug: 
	$(CC) $(srcFiles) $(COMPILER_FLAGS) $(LINKER_FLAGS) -DDEBUG

.PHONY: clean
clean:
	rm -f $(objFiles) a.out

