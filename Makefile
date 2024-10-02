CFLAGS = -Wall -Wextra -Wno-missing-braces -std=c11
CC = gcc
BUILD = build

all: tvm tasm

run: tvm
	./$(BUILD)/tvm.exe
tvm:
	if not exist $(BUILD) mkdir $(BUILD)
	$(CC) $(CFLAGS) ./src/tvm.c ./src/tci.c -I "./include" -I "./extern/libffi-win32/include" -o ./$(BUILD)/tvm.exe -L ./extern/libffi-win32/lib -llibffi

tasm:
	if not exist $(BUILD) mkdir $(BUILD)
	$(CC) $(CFLAGS) ./src/tasm.c -I ./include -I ./extern/stb/include -o ./$(BUILD)/tasm.exe