all: rom.bin dectest.bin

rom.bin: main.o
	ld65 $^ -C rom.cfg -o rom.bin

dectest.bin: dectest.o
	ld65 $^ -C dectest.cfg -o dectest.bin

%.o: %.s
	ca65 $< -l $<.lst

