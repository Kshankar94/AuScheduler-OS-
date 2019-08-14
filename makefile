commandline: aubatch.c command.h
	gcc -o aubatch aubatch.c -lpthread

command.o: command.c command.h
	cc -c command.c
