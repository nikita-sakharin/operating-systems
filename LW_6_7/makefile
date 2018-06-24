CC=gcc
CFLAGS=-c -std=c11 -Werror -pedantic -Wall -Wextra -g
LDFLAGS=
LIBS=-lzmq
SOURCES=./bin_tree/bin_tree.c
SOURCES_SERVER=server.c
SOURCES_CLIENT=client.c
OBJECTS=$(SOURCES:.c=.o)
OBJECTS_SERVER=$(SOURCES_SERVER:.c=.o)
OBJECTS_CLIENT=$(SOURCES_CLIENT:.c=.o)
EXECUTABLE=server client

all: $(SOURCES) $(SOURCES_SERVER) $(SOURCES_CLIENT) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS) $(OBJECTS_SERVER) $(OBJECTS_CLIENT)
	$(CC) $(LDFLAGS) $(OBJECTS) $@.o $(LIBS) -o $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	-rm -frd $(OBJECTS) $(OBJECTS_SERVER) $(OBJECTS_CLIENT) $(EXECUTABLE)