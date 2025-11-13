CC = gcc
CFLAGS = -Wall -O2

all: Task8

Task8: Task8.c
	$(CC) $(CFLAGS) -o Task8 Task8.c

run:
	@echo "Running tests..."
	@./Task8 10
	@./Task8 100
	@./Task8 1000
	@./Task8 10000
	@./Task8 100000
	@./Task8 1000000
	@./Task8 10000000

clean:
	rm -f Task8
