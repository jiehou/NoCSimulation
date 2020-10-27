CC     := g++
OPT    := -O3 # -O3
# DEBUG  = -g
OTHER  := -Wall -DSC_NO_WRITE_CHECK -Wno-deprecated --std=c++11
CFLAGS := $(OPT) $(OTHER)
LIBS := -pthread

all: pgenerator

pgenerator: pgenerator.o
	$(CC) pgenerator.o -o pgenerator $(LIBS)

pgenerator.o: pgenerator.cpp
	$(CC) $(CFLAGS) -c pgenerator.cpp -o pgenerator.o

clean:
	rm -f *.o pgenerator