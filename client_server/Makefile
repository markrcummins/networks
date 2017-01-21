OBJECT1 = unixclient.c
OBJECT2 = unixserver.c
CC = gcc
BC = gcc -pthread
UBC = gcc -lpthread -lsocket -lnsl
UCC = gcc -lsocket
all: server client

server: $(OBJECT2)
	$(BC) $(OBJECT2) -o server

client: $(OBJECT1)
	$(CC) $(OBJECT1) -o client