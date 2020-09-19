CC = gcc
CFLAGS = -Wall -Wextra -pedantic
CFLAGS_DEBUG = -g -O0 -DDEBUG
INCL = -I./include
SRCS = mem.c cpu.c logger.c regs.c rom.c ints.c gpu.c display.c main.c debug.c input.c joypad.c mem_rtc.c timer.c
PKG = $$(pkg-config --libs allegro-5 allegro_primitives-5)
OBJS = $(SRCS:.c=.o)
BIN = gbc

all: gbc_debug

gbc_debug: CFLAGS += $(CFLAGS_DEBUG)
gbc_debug: gbc

gbc: $(OBJS)
	$(CC) $(CFLAGS) $(INCL) -o $(BIN) $(OBJS) $(PKG)

.c.o:
	$(CC) $(CFLAGS) $(INCL) -c $< -o $@

.PHONY: clean
clean:
	rm -f *.o
