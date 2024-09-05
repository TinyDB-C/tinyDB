CC = gcc

#CFLAGS = -oFast

SRCS = $(wildcard *.c)
OBJS = $(SRCS:.c=.o)
EXEC = tinydb

$(EXEC): $(OBJS)
	$(CC) $(OBJS) -o $(EXEC)

-include $(OBJS:.o=.d)

%.o: %.c
#	$(CC) $(CFLAGS) -c $< -o $@
	$(CC) -MMD -c $< -o $@

clean:
	rm -f $(OBJS) $(OBJS:.o=.d) $(EXEC)