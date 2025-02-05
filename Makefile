CFLAGS = -g -Wall
CC = g++

all: httpServer httpClient

httpServer: httpServer.o
	$(CC) $(CFLAGS) -o httpServer httpServer.o

httpClient: httpClient.o
	$(CC) $(CFLAGS) -o httpClient httpClient.o

httpServer.o: httpServer.cpp
	$(CC) $(CFLAGS) -c httpServer.cpp

httpClient.o: httpClient.cpp
	$(CC) $(CFLAGS) -c httpClient.cpp

.PHONY: clean run

clean:
	rm -f httpServer httpClient httpServer.o httpClient.o

run: all
	./httpServer & sleep 1 && ./httpClient