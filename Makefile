CFLAGS=-Wall -g -I/usr/include/libxml2 -I/user/include/gsl 
LIBS=-lxml2 -lz -lm -lgsl

fucheck: fucheck.o
	gcc ${CFLAGS} ${LIBS} fucheck.o -o fucheck

fucheck.o: fucheck.c
	gcc -c ${CFLAGS} ${LIBS} fucheck.c

clean:
	rm -f *.o fucheck
