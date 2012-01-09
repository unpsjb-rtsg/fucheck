CFLAGS=-Wall -g

fucheck: fucheck.o
	gcc fucheck.o -o fucheck

fucheck.o: fucheck.c
	gcc -c fucheck.c

clean:
	rm -f *.o fucheck
