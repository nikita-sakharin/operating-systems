CC=gcc
CFLAGS=-c -std=c11 -Werror -pedantic -Wall -Wextra -g -D_FILE_OFFSET_BITS=64 -D_XOPEN_SOURCE=500
LDFLAGS=
LIBS=-lm
SOURCES=./list/list.c ./stack/stack.c ./queue/queue.c v_mem.c CP.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=CP

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) $(LIBS) -o $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -frd $(OBJECTS) $(EXECUTABLE)