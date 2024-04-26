main: main.o
	gcc -pthread -o s-talk main.o
	
main.o: main.c
	gcc -lpthread -std=gnu1x -D_GNU_SOURCE -c -g main.c
clean:
	rm s-talk main.o[]
