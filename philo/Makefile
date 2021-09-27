CC=gcc
RM=rm -frd
CFLAGS=-Wall -Walloc-zero -Walloca -Wanalyzer-too-complex -Warith-conversion \
    -Warray-bounds=2 -Wbad-function-cast -Wc++-compat -Wcast-align=strict \
    -Wcast-qual -Wconversion -Wdate-time -Wdisabled-optimization \
    -Wdouble-promotion -Wduplicated-branches -Wduplicated-cond -Werror -Wextra \
    -Wfatal-errors -Wfloat-equal -Wformat=2 -Wformat-overflow=2 \
    -Wformat-signedness -Wformat-truncation=2 -Wimplicit-fallthrough=5 \
    -Winit-self -Winline -Wlogical-op -Wmissing-declarations \
    -Wmissing-include-dirs -Wmissing-noreturn -Wmissing-prototypes \
    -Wnull-dereference -Wold-style-definition -Wpacked -Wpedantic \
    -Wredundant-decls -Wshadow=global -Wshift-overflow=2 -Wstrict-overflow=5 \
    -Wstrict-prototypes -Wstringop-overflow=4 -Wswitch-default -Wswitch-enum \
    -Wundef -Wunreachable-code -Wunused-const-variable=2 -Wunused-macros \
    -Wvector-operation-performance -Wvla -Wwrite-strings -fanalyzer \
    -pedantic-errors -std=c18
LDFLAGS=
LDLIBS=-lm
SOURCES=main.c philo.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=main

all: $(SOURCES) $(EXECUTABLE)

debug: CFLAGS+=-Og -g
debug: all
release: CFLAGS+=-DNDEBUG -O3 -flto -march=native -mtune=native -s
release: LDFLAGS+=-O3 -flto -march=native -mtune=native -s
release: all

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) $(LDLIBS) -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) $(OBJECTS) $(EXECUTABLE)
