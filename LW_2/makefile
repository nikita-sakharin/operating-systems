CC=gcc
CFLAGS=-c -std=c11 -Werror -pedantic -Wall -Wextra -O3
LDFLAGS=
SOURCES=LW_2.c child.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=$(SOURCES:.c=)

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $@.o -o $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -frd $(OBJECTS) $(EXECUTABLE)