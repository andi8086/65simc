all: rom.bin dectest.bin cmptest.bin

rom.bin: main.o
	ld65 $^ -C rom.cfg -o rom.bin

dectest.bin: dectest.o
	ld65 $^ -C dectest.cfg -o dectest.bin

cmptest.bin: cmptest.o
	ld65 $^ -C cmptest.cfg -o cmptest.bin

%.o: %.s
	ca65 $< -l $<.lst

