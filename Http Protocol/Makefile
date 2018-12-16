CC=g++
CFLAGS= -Wall -W
all: server client

server: server/server.cpp
	$(CC) $(CFLAGS) server/server.cpp server/requestHandler.cpp lib/HttpMessage.cpp lib/fileHandler.cpp -o myserver -pthread
	
client: client/client.cpp
	$(CC) $(CFLAGS) client/client.cpp lib/HttpMessage.cpp lib/fileHandler.cpp -o myclient -pthread

clean:
	rm -f myserver.o myclient.o
