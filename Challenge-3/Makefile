CC = g++
CCFLAGS = -Wall
EFLAGS = -I/usr/include/eigen3/
CIMGFLAGS = -L/usr/X11R6/lib -lm -lpthread -lX11
MAIN = repairJPEG
OBJ = $(MAIN).o

driver.out: $(OBJ)
	$(CC) $(CCFLAGS) -o driver.out $(OBJ)

$(MAIN).o: $(MAIN).cpp parseKDB.h lfsr.h
	$(CC) $(CCFLAGS) -c $(MAIN).cpp

.PHONY:
run:
	./driver.out magic.kdb input.bin

.PHONY:
valrun:
	valgrind ./driver.out magic.kdb input.bin

.PHONY:
test:
	./driver.out magic.kdb input.bin test

.PHONY:
clean:

	rm *.o
	rm driver.out
	rm *~*
