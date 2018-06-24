CC=gcc
CFLAGS=-c -std=c11 -D_XOPEN_SOURCE=500 -Werror -pedantic -Wall -Wextra
LDFLAGS=
LIBS=
SOURCES=md5.c file.c LW_4.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=LW_4

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) $(LIBS) -o $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -frd $(OBJECTS) $(EXECUTABLE)