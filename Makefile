COMPILER = gcc

all: executive task.o

no-dmiss: executive.o no-dmiss.o
	gcc -o executive executive.o no-dmiss.o -lpthread

no-dmiss-sporadic: executive.o no-dmiss-sporadic.o
	gcc -o executive executive.o no-dmiss-sporadic.o -lpthread

dmiss-task: executive.o dmiss-task.o
	gcc -o executive executive.o dmiss-task.o -lpthread

dmiss-sporadic: executive.o dmiss-sporadic.o
	gcc -o executive executive.o dmiss-sporadic.o -lpthread

no-dmiss.o: no-dmiss.c task.h
	$(COMPILER) no-dmiss.c -Wall -c -o no-dmiss.o

no-dmiss-sporadic.o: no-dmiss-sporadic.c task.h
	$(COMPILER) no-dmiss-sporadic.c -Wall -c -o no-dmiss-sporadic.o

dmiss-task.o: dmiss-task.c task.h
	$(COMPILER) dmiss-task.c -Wall -c -o dmiss-task.o

dmiss-sporadic.o: dmiss-sporadic.c task.h
	$(COMPILER) dmiss-sporadic.c -Wall -c -o dmiss-sporadic.o

executive.o: executive.c task.h
	$(COMPILER) executive.c -Wall -lpthread -c -o executive.o

clean:
	rm -f executive *.o *~
