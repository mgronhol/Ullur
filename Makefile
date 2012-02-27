CC = g++
FLAGS = -O3 -Wall -pedantic -std=c++0x

all: libhash Ullur

libhash:
	$(CC) $(FLAGS) -c libhash.cpp -o libhash.o

Ullur: libhash
	$(CC) $(FLAGS) ullur.cpp -o ullur libhash.o

benchmark: libhash
	$(CC) $(FLAGS) benchmark.cpp -o benchmark libhash.o

clean:
	rm *.o ullur
	rm benchmark
