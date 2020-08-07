CC = gcc
CFLAGS = -g -Wall -Wextra -pedantic -O0
INCL = -I./include
SRCS = cpu.c logger.c regs.c rom.c ints.c main.c mem.c debug.c
OBJS = $(SRCS:.c=.o)
BIN = gbc

all: gbc_debug

gbc_debug: CFLAGS += -DDEBUG
gbc_debug: gbc

gbc: $(OBJS)
	$(CC) $(CFLAGS) $(INCL) -o $(BIN) $(OBJS)

.c.o:
	$(CC) $(CFLAGS) $(INCL) -c $< -o $@

.PHONY: clean
clean:
	rm *.o $(BIN)
