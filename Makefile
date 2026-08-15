CC = cc
CFLAGS = -Wall -Wextra -pedantic -std=c99
OBJS = main.o terminal.o syntax.o row.o editor_ops.o file_io.o find.o abuf.o output.o input.o

swan: $(OBJS)
	$(CC) $(CFLAGS) -o swan $(OBJS)

%.o: %.c swan.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f swan $(OBJS)

.PHONY: clean
