FLAGS = -c -g3 -Wall -O3 -pthread
OBJS = *.o
TARGETS = dataserver
CC = g++

dataserver: dataserver.o Queue.o
	g++  -pthread dataserver.o Queue.o -o dataserver

dataserver.o: ./dataserver.cpp
	$(CC) $(FLAGS) ./dataserver.cpp

Queue.o : ./Queue.cpp ./Queue.h
	$(CC) $(FLAGS)  ./Queue.cpp

clean:
	rm -rf $(TARGETS) $(OBJS)
