all: rom.bin

rom.bin: main.o
	ld65 $^ -C rom.cfg -o rom.bin

%.o: %.s
	ca65 $<

