p = generate
f = -g -std=c++0x
c = g++

all: Main.o
	$c $f -o $p Main.o

Main.o: Main.cpp
	$c $f -c Main.cpp

clean:
	rm *.o *.txt $p
