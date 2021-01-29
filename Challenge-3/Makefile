CC = g++
CCFLAGS = -Wall
EFLAGS = -I/usr/include/eigen3/
CIMGFLAGS = -L/usr/X11R6/lib -lm -lpthread -lX11
MAIN = repairJPEG
OBJ = $(MAIN).o md5.o

all: driver.exe

driver.exe: $(OBJ)
	$(CC) $(CCFLAGS) -o driver.exe $(OBJ)

$(MAIN).o: $(MAIN).cpp md5.h parseKDB.h
	$(CC) $(CCFLAGS) -c $(MAIN).cpp

md5.o: md5.cpp md5.h
	$(CC) $(CCFLAGS) -c md5.cpp

.PHONY:
run:
	./driver.exe magic.kdb input.bin

.PHONY:
valrun:
	valgrind ./driver.exe magic.kdb input.bin

.PHONY:
test:
	./driver.exe magic.kdb input.bin test

.PHONY:
clean:

	rm *.o
	rm driver.exe
	rm *~*
