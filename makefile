CC = g++
CFLAGS = -Wall -Werror -g -std=c++17

EXE = bin/myfind

all: $(EXE)

rebuild: clean $(EXE)

$(EXE): main.cpp | bin
	$(CC) $(CFLAGS) -o $@ $^

bin:
	mkdir -p $@

clean:
	rm -rf bin
