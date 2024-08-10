CFLAGS = -Wall -Wextra -Wno-missing-braces -std=c11
CC = gcc

all: tvm tasm

run: tvm
	./build/tvm.exe
tvm:
	if not exist build mkdir build
	$(CC) $(CFLAGS) ./src/tvm.c -I ./include -o ./build/tvm.exe

tasm:
	if not exist build mkdir build
	$(CC) $(CFLAGS) ./src/tasm.c -I ./include -I ./extern/stb/include -o ./build/tasm.exe