OPTIONS = -Wall -lpthread

all: executive

executive: executive.c
	gcc executive.c $(OPTIONS) -o executive

clean:
	rm executive *.o
