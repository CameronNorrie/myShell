CC = gcc
CFLAGS = -Wpedantic -std=gnu99 -g

all:
	$(CC) $(CFLAGS) -c myShell.c -o myShell.o
	$(CC) $(CFLAGS)  myShell.o -o myShell

myShell.o: myShell.c
	$(CC) $(CFLAGS) -c myShell.c -o myShell.o

myShell: myShell.o
	$(CC) $(CFLAGS)  myShell.o -o myShell

clean:
	rm myShell.o myShell