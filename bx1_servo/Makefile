CC = g++
MRAALIBS = -lmraa

.PHONY: all clean servo

all: servo

clean:
	rm -f ./*.o
	rm -f servo

servo: servo.o
	$(CC) -O4 $(MRAALIBS) -o $@ $^

%.o: %.c
	$(CC) -Wall -g -c $<
