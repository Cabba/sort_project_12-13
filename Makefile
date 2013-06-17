OPTIONS = -Wall -lpthread
COMPILER = gcc

no-dmiss: executive no-dmiss.o

dmiss-task: executive dmiss-task.o

dmiss-sporadic: executive dmiss-sporadic.o

executive: executive.c task.o
	$(COMPILER) executive.c $(OPTIONS) -o executive

no-dmiss.o: no-dmiss.c
	$(COMPILER) no-dmiss.c -Wall -c -o no-dmiss.o

dmiss-task.o: dmiss-task.c
	$(COMPILER) dmiss-task.c -Wall -c dmiss-task.o

dmiss-sporadic.o: dmiss-sporadic.c
	$(COMPILER) dmiss-sporadic.c -Wall -c dmiss-sporadic.o

clean:
	rm -f executive *.o *~
