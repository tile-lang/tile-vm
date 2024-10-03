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
	$(CC) $(CFLAGS) ./src/tasm.c ./src/tasmc.c -I ./include -I ./extern/stb/include -o ./$(BUILD)/tasm.exe

tasmc: win64

# so fucking experimental that I just put the path of my linker

win64: out.o set_vcvars link
#-----------------#
out.o:
	nasm -f win64 $(BUILD)/out.asm -o $(BUILD)/out.o
set_vcvars:
	"C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsx86_amd64.bat"
link:
	link /entry:_start /subsystem:console /DEBUG /OUT:$(BUILD)/out.exe $(BUILD)/out.o kernel32.lib