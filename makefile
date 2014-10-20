CC=gcc

#CFLAGS = -ansi -c -Wall -pedantic 
CFLAGS = -c  -g
all: mcast start_mcast

mcast: mcast.o recv_dbg.o 
	    $(CC) -o mcast mcast.o recv_dbg.o

start_mcast: start_mcast.o
	    $(CC) -o start_mcast start_mcast.o

recv_dbg: recv_dbg.o
	    $(CC) -o recv_dbg recv_dbg.o
clean:
	rm *.o
	rm mcast
	rm start_mcast

%.o:    %.c
	$(CC) $(CFLAGS) $*.c

