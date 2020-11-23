CC = gcc
CFLAGS = -Wall -Wextra -pedantic
CFLAGS_DEBUG = -g -O0 -DDEBUG
CFLAGS_JITTER = -DJITTER
INCL = -I./include
SRCS = cpu.c debug.c display.c events.c gpu.c input.c ints.c joypad.c \
	logger.c main.c mem.c mem_rtc.c regs.c rom.c sys.c timer.c sound.c
LBR = -pthread -lSDL2
OBJS = $(SRCS:.c=.o)
BIN = gbc

all: gbc_jitter

gbc_jitter: CFLAGS += $(CFLAGS_JITTER)
gbc_jitter: gbc

gbc_debug: CFLAGS += $(CFLAGS_DEBUG)
gbc_debug: gbc

gbc: $(OBJS)
	$(CC) $(CFLAGS) $(INCL) -o $(BIN) $(OBJS) $(LBR)

.c.o:
	$(CC) $(CFLAGS) $(INCL) -c $< -o $@

.PHONY: clean
clean:
	rm -f *.o
