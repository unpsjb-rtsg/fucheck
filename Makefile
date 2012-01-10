CFLAGS=-Wall -g -I/usr/include/libxml2
LIBS=-lxml2 -lz -lm

fucheck: fucheck.o
	gcc ${CFLAGS} ${LIBS} fucheck.o -o fucheck

fucheck.o: fucheck.c
	gcc -c ${CFLAGS} ${LIBS} fucheck.c

clean:
	rm -f *.o fucheck
