CC=gcc
CFLAGS=-c -std=c11 -Werror -pedantic -Wall -Wextra -O3
LDFLAGS=
LIBS=-pthread
SOURCES=./stack/stack.c ./b_search/b_search.c tim_sort.c LW_3.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=LW_3

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) $(LIBS) -o $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -frd $(OBJECTS) $(EXECUTABLE)