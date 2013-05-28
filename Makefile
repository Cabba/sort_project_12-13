OPTIONS = -Wall -lpthread

all: executive

executive: executive.c executive.h
	gcc executive.c $(OPTIONS) -o executive

clean:
	rm -f executive *.o *~
