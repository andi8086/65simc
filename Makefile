CCFLAGS=-Wno-deprecated-declarations \
        -pthread \
        -lm \
        -o sim \
        $(shell pkg-config --cflags --libs gtk+-3.0)

SRCS=sim.c \
     funcs.c \
     6502.c \
	 mem.c \
     jsmn/jsmn.c \
     gui.c \
     cfg.c \
     clock_sync.c \
     6522.c \
     6551.c \
     AY-3-8910.c

.PHONY: sim

all: sim

sim:
	gcc $(CCFLAGS) $(SRCS) 

ignore:
	gcc $(CCFLAGS) -Wl,--unresolved-symbols=ignore-all $(SRCS)

clean:
	@rm -f ./sim
	
