CC = g++
CCFLAGS = -Wall
EFLAGS = -I/usr/include/eigen3/
CIMGFLAGS = -L/usr/X11R6/lib -lm -lpthread -lX11
MAIN = testDriver
OBJ = $(MAIN).o

driver.out: $(OBJ)
	$(CC) $(CCFLAGS) -o driver.out $(OBJ)

$(MAIN).o: $(MAIN).cpp lfsr.h
	$(CC) $(CCFLAGS) -c $(MAIN).cpp

.PHONY:
run:
	./driver.out

.PHONY:
valrun:
	valgrind ./driver.out

.PHONY:
test:
	./driver.out test

.PHONY:
clean:

	rm *.o
	rm driver.out
	rm *~*
