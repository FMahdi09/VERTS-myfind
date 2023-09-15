CC = g++
CFLAGS = -Wall -Werror -g

EXE = bin/main

all: $(EXE)

rebuild: clean $(EXE)

$(EXE): main.cpp | bin
	$(CC) $(CFLAGS) -o $@ $^

bin:
	mkdir -p $@

clean:
	rm -rf bin
