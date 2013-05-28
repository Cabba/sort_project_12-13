OPTIONS = -Wall -lpthread

all: executive task.o

executive: executive.c executive.h task.o
	gcc executive.c $(OPTIONS) -o executive

task.o: task-example.c
	gcc task-example.c -Wall -c -o task.o
clean:
	rm -f executive *.o *~
