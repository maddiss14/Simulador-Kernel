TARGET := main

SRCS := clock.c main.c scheduler.c timer.c process_generator.c machine.c loader.c memoria.c

CC = gcc
CFLAGS = -O2 -pthread

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS)

clean:
	rm -f $(TARGET)

