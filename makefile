CC=gcc

#CFLAGS = -ansi -c -Wall -pedantic -g
CFLAGS = -c  -g
all: mcast

mcast: mcast.o 
	    $(CC) -o mcast mcast.o

clean:
	rm *.o
	rm mcast

%.o:    %.c
	$(CC) $(CFLAGS) $*.c
