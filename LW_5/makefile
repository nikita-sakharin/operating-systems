CC=gcc
CFLAGS=-std=c11 -Werror -pedantic -Wall -Wextra -O3

all: LW_5_static LW_5_dynamic

LW_5_static: LW_5_static.o libstack.a
	gcc LW_5_static.o -L. -lstack -o LW_5_static

LW_5_dynamic: LW_5_dynamic.o libstack.so
	gcc LW_5_dynamic.o -L. -lstack -Wl,-rpath,. -o LW_5_dynamic -ldl

LW_5_static.o: LW_5_static.c
	$(CC) -c $(CFLAGS) LW_5_static.c

LW_5_dynamic.o: LW_5_dynamic.c
	$(CC) -c $(CFLAGS) LW_5_dynamic.c

libstack.a: ./stack/stack.o
	ar cr libstack.a ./stack/stack.o

libstack.so: ./stack/stack.o
	gcc -shared ./stack/stack.o -o libstack.so

./stack/stack.o: ./stack/stack.c
	$(CC) -c $(CFLAGS) -fPIC ./stack/stack.c -o ./stack/stack.o

clean:
	rm -frd LW_5_static LW_5_static.o libstack.a LW_5_dynamic LW_5_dynamic.o libstack.so ./stack/stack.o