CC = gcc

cache: cache.c
	$(CC) -o cache cache.c

.PHONY: clean
clean:
	rm -f cache