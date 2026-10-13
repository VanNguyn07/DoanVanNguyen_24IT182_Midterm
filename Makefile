CC = cc
CFLAGS = -std=c99 -Wall -Wextra -Wpedantic -g
OBJS = main.o listing.o options.o operands.o traversal.o display.o mode.o long_format.o sizes.o timestamps.o sorting.o names.o

all: ls

ls: $(OBJS)
	$(CC) $(CFLAGS) -o ls $(OBJS)

main.o: main.c operands.h options.h
	$(CC) $(CFLAGS) -c main.c

listing.o: listing.c listing.h display.h sorting.h options.h
	$(CC) $(CFLAGS) -c listing.c

options.o: options.c options.h sizes.h
	$(CC) $(CFLAGS) -c options.c

operands.o: operands.c operands.h traversal.h display.h sorting.h options.h names.h
	$(CC) $(CFLAGS) -c operands.c

traversal.o: traversal.c traversal.h listing.h sorting.h options.h names.h
	$(CC) $(CFLAGS) -c traversal.c

display.o: display.c display.h long_format.h mode.h sizes.h options.h names.h
	$(CC) $(CFLAGS) -c display.c

mode.o: mode.c mode.h
	$(CC) $(CFLAGS) -c mode.c

long_format.o: long_format.c long_format.h mode.h sizes.h timestamps.h options.h names.h
	$(CC) $(CFLAGS) -c long_format.c

clean:
	rm -f ls $(OBJS)

.PHONY: all clean

sizes.o: sizes.c sizes.h options.h
	$(CC) $(CFLAGS) -c sizes.c

timestamps.o: timestamps.c timestamps.h options.h
	$(CC) $(CFLAGS) -c timestamps.c

sorting.o: sorting.c sorting.h timestamps.h options.h
	$(CC) $(CFLAGS) -c sorting.c

names.o: names.c names.h options.h
	$(CC) $(CFLAGS) -c names.c
