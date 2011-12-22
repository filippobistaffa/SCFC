.PHONY: 
	
all:
	gcc -Icmph -Ofast *.c -lpthread -o SCFC
