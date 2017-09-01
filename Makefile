sim: sim.c funcs.c 6502.c
	gcc -pthread -lm -o sim -g $(shell pkg-config --cflags --libs gtk+-3.0) sim.c funcs.c 6502.c
