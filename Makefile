TARGET := main

SRCS := clock.c main.c scheduler.c timer.c process_generator.c machine.c loader.c memoria.c process_manager.c

CC = gcc
CFLAGS = -O0 -g -pthread

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS)

clean:
	rm -f $(TARGET)

