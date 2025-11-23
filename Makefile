CC = gcc
CFLAGS = -Wall -Wextra -Werror -std=c11 -Iheaders
TARGET = connect4

SRCS = \
	main.c \
	board.c \
	bot_manager.c \
	bots/bot_utils.c \
	bots/bot_easy.c \
	bots/bot_medium.c \
	bots/bot_hard.c

OBJS = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
