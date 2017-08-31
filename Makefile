sim: sim.c funcs.c 6502.c
	gcc -pthread -lm -o sim -g sim.c funcs.c 6502.c
