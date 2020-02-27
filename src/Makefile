CC = gcc
CFLAGS = -g -Wall -Wextra -pedantic -O0
INCL = -I./include
SRCS = cpu.c regs.c rom.c main.c
OBJS = $(SRCS:.c=.o)
BIN = gbc

all: gbc

gbc: $(OBJS)
	$(CC) $(CFLAGS) $(INCL) -o $(BIN) $(OBJS)

.c.o:
	$(CC) $(CFLAGS) $(INCL) -c $< -o $@

.PHONY: clean
clean:
	rm *.o $(BIN)
