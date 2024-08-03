CFLAGS = -Wall -Wextra -Wno-missing-braces

make: tvm

run: tvm
	./build/tvm.exe
tvm:
	gcc $(CFLAGS) ./src/tvm.c -I ./include -o ./build/tvm.exe