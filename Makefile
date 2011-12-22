.PHONY: 
	
all:
	gcc -Icmph -Ofast *.c cmph/*.c -lpthread -lm -o SCFC
