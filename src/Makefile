CC = gcc
CFLAGS = -g -Wall -Wextra -pedantic -O0
INCL = -I./include
SRCS = mem.c cpu.c logger.c regs.c rom.c ints.c gpu.c display.c main.c debug.c
PKG = $$(pkg-config --libs allegro-5.0)
OBJS = $(SRCS:.c=.o)
BIN = gbc

all: gbc_debug

gbc_debug: CFLAGS += -DDEBUG
gbc_debug: gbc

gbc: $(OBJS)
	$(CC) $(CFLAGS) $(INCL) -o $(BIN) $(OBJS) $(PKG)

.c.o:
	$(CC) $(CFLAGS) $(INCL) -c $< -o $@

.PHONY: clean
clean:
	rm -f *.o $(BIN)
