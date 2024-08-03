make: tvm

run: tvm
	./build/tvm.exe
tvm:
	gcc ./src/tvm.c -I ./include -o ./build/tvm.exe