CFLAGS = -Wall -Wextra -Wno-missing-braces -std=c11
CC = gcc
BUILD_DIR = build
EXAMPLES_DIR = examples
EXAMPLES_BIN_DIR = $(EXAMPLES_DIR)/build
LIBFFI = extern/libffi-mingw32
# 	  or extern/libffi-mingw64

all: tvm tasm

run: tvm
	./$(BUILD_DIR)/tvm.exe
tvm:
	if not exist $(BUILD_DIR) mkdir $(BUILD_DIR)
	$(CC) $(CFLAGS) ./src/tvm.c ./src/tci.c -I "./include" -I "./$(LIBFFI)/include" -I ./extern/stb/include -o ./$(BUILD_DIR)/tvm.exe -L ./$(LIBFFI)/lib -llibffi

tasm:
	if not exist $(BUILD_DIR) mkdir $(BUILD_DIR)
	$(CC) $(CFLAGS) ./src/tasm.c ./src/tasmc.c -I ./include -I ./extern/stb/include -o ./$(BUILD_DIR)/tasm.exe


# Get all the .tasm files in the examples directory
TASM_FILES = $(wildcard $(EXAMPLES_DIR)/*.tasm)

# Generate .bin files in the EXAMPLES_BIN_DIR with the same base name as .tasm files
BIN_FILES = $(patsubst $(EXAMPLES_DIR)/%.tasm, $(EXAMPLES_BIN_DIR)/%.bin, $(TASM_FILES))

# Rule to convert all .tasm files to .bin files in the EXAMPLES_BIN_DIR
examples: $(BIN_FILES)

# Rule to create the EXAMPLES_BIN_DIR if it doesn't exist
$(EXAMPLES_BIN_DIR):
	mkdir -p $(EXAMPLES_BIN_DIR)

# Rule to convert a specific .tasm file to a .bin file inside EXAMPLES_BIN_DIR
$(EXAMPLES_BIN_DIR)/%.bin: $(EXAMPLES_DIR)/%.tasm | $(EXAMPLES_BIN_DIR)
	$(BUILD_DIR)/tasm $< -o $@



tasmc: win64

# so fucking experimental that I just put the path of my linker

win64: out.o set_vcvars link
#-----------------#
out.o:
	nasm -f win64 $(BUILD_DIR)/out.asm -o $(BUILD_DIR)/out.o
set_vcvars:
	"C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsx86_amd64.bat"
link:
	link /entry:_start /subsystem:console /DEBUG /OUT:$(BUILD_DIR)/out.exe $(BUILD_DIR)/out.o kernel32.lib