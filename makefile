FLAGS = -g -std=c++0x
C = g++
OUT = program3

all:	lfs

lfs:	Driver.o LFS.o
	$(C) $(FLAGS) Driver.o LFS.o -o $(OUT)

Driver.o:	Driver.cpp
	$(C) $(FLAGS) -c Driver.cpp

LFS.o:	LFS.cpp
	$(C) $(FLAGS) -c LFS.cpp

clean:
	rm -rf *.o $(OUT)
