CC = g++
FLAGS = -O3 -Wall -pedantic -std=c++0x

all: libhash Ullur

libhash:
	$(CC) $(FLAGS) -c libhash.cpp -o libhash.o

Ullur: libhash
	$(CC) $(FLAGS) ullur.cpp -o ullur libhash.o

clean:
	rm *.o ullur
