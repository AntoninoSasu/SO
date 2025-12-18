CC = gcc
CFLAGS = -O0 -g -pthread
LDFLAGS = -O0 -g -pthread

all: discoMOD3

%.o: %.c Makefile
	$(CC) $(CFLAGS) -c -o $@ $<

discoMOD3: discoMOD3.o
	$(CC) $(LDFLAGS) -o $@ $<

.PHONY: clean all


clean:
	-rm discoMOD3 discoMOD3.o
