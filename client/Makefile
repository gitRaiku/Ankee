all: build

.PHONY: build client server

CC = gcc
DATE := $(shell date "+%Y-%m-%d")
COMPILE_FLAGS = -Og -g -ggdb3 -march=native -mtune=native -Wall -D_FORTIFY_SOURCE=2 -fmodulo-sched
# COMPILE_FLAGS = -Ofast -ggdb3 -march=native -mtune=native -Wall -D_FORTIFY_SOURCE=2 -fmodulo-sched
INCLUDE_FLAGS = 
LIBRARY_FLAGS = -lncursesw -lpanelw
 DEFINE_FLAGS = -D_DEFAULT_SOURCE -D_XOPEN_SOURCE=600

build:
	$(CC) $(COMPILE_FLAGS) $(DEFINE_FLAGS) $(LIBRARY_FLAGS) -o out a.c

install: build
	cp out /usr/local/bin/ankeec

run: build
	./out "きのこ人間" "/tmp/mata"
