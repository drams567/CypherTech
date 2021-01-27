CC = g++
CCFLAGS = -Wall
EFLAGS = -I/usr/include/eigen3/
CIMGFLAGS = -L/usr/X11R6/lib -lm -lpthread -lX11
MAIN = parseKDB
OBJ = $(MAIN).o

driver.out: $(OBJ)
	$(CC) $(CCFLAGS) -o driver.out $(OBJ)

$(MAIN).o: $(MAIN).cpp lfsr.h
	$(CC) $(CCFLAGS) -c $(MAIN).cpp

.PHONY:
run:
	./driver.out store.kdb

.PHONY:
valrun:
	valgrind ./driver.out store.kdb

.PHONY:
test:
	./driver.out store.kdb test

.PHONY:
clean:

	rm *.o
	rm driver.out
	rm *~*
