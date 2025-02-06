CFLAGS = -g -Wall -pthread
LDFLAGS = -pthread
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

.PHONY: clean run_server run_client

clean:
	rm -f httpServer httpClient httpServer.o httpClient.o

# Start the server separately
server: httpServer
	./httpServer

# Start the client separately
client: httpClient
	./httpClient
