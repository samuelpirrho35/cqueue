CC = gcc
AR = ar

CFLAGS = -Wall -Wextra -Iinclude

TARGET = libcqueue.a
OBJ = src/cqueue.o

PREFIX ?= /usr/local

all: $(TARGET)

$(TARGET): $(OBJ)
	$(AR) rcs $@ $^

src/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

example: $(TARGET)
	$(CC) $(CFLAGS) example/main.c -L. -lcqueue -o example/cqueue_example

install: $(TARGET)
	install -d $(PREFIX)/include/cqueue
	install -d $(PREFIX)/lib
	install -m 644 include/cqueue/cqueue.h $(PREFIX)/include/cqueue/
	install -m 644 $(TARGET) $(PREFIX)/lib/

clean:
	rm -f $(OBJ) $(TARGET) example/cqueue_example