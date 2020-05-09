srcFiles = $(wildcard *.c)
objFiles = $(srcFiles:.c=.o)
debugFiles = $(srcFiles:.c=DEBUG.o)

CC = gcc

COMPILER_FLAGS = -Wall -Wpedantic -std=c11

LINKER_FLAGS = -lSDL2 

# target
%.o : %.c
	$(CC) $< $(COMPILER_FLAGS)

all : $(objFiles) 
	$(CC) $^ $(LINKER_FLAGS) 

%DEBUG.o : %.c
	$(CC) -g $< $(COMPILER_FLAGS) -DDEBUG

debug : debugFiles
	$(CC) $^ $(LINKER_FLAGS)

.PHONY: clean
clean:
	rm -f $(objFiles) $(debugFiles) a.out

