CCFLAGS=-Wno-deprecated-declarations \
        -pthread \
        -lm \
        -o sim \
        $(shell pkg-config --cflags --libs gtk+-3.0)

SRCS=sim.c \
     funcs.c \
     6502.c \
     jsmn/jsmn.c \
     gui.c \
     cfg.c

all: sim

sim:
	gcc $(CCFLAGS) $(SRCS) 

clean:
	@rm -f ./sim
	
