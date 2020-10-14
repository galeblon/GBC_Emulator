CC = gcc
CFLAGS = -Wall -Wextra -pedantic
CFLAGS_DEBUG = -g -O0 -DDEBUG
INCL = -I./include
SRCS = cpu.c debug.c display.c gpu.c input.c ints.c joypad.c \
	   logger.c main.c mem.c mem_rtc.c regs.c rom.c sys.c timer.c
PKG = $$(pkg-config --libs allegro-5 allegro_primitives-5)
LBR = -lpthread
OBJS = $(SRCS:.c=.o)
BIN = gbc

all: gbc_debug

gbc_debug: CFLAGS += $(CFLAGS_DEBUG)
gbc_debug: gbc

gbc: $(OBJS)
	$(CC) $(CFLAGS) $(INCL) -o $(BIN) $(OBJS) $(LBR) $(PKG)

.c.o:
	$(CC) $(CFLAGS) $(INCL) -c $< -o $@

.PHONY: clean
clean:
	rm -f *.o
