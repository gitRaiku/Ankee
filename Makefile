all: build

.PHONY: build client server

CC = gcc
DATE := $(shell date "+%Y-%m-%d")
COMPILE_FLAGS = -Og -g -ggdb3 -march=native -mtune=native -Wall -D_FORTIFY_SOURCE=2 -fmodulo-sched
# COMPILE_FLAGS = -Ofast -ggdb3 -march=native -mtune=native -Wall -D_FORTIFY_SOURCE=2 -fmodulo-sched
INCLUDE_FLAGS = 
LIBRARY_FLAGS = -lncursesw -lpanelw
 DEFINE_FLAGS = -D_DEFAULT_SOURCE -D_XOPEN_SOURCE=600
PWD := $(shell pwd)

build:
	$(CC) $(COMPILE_FLAGS) $(DEFINE_FLAGS) $(LIBRARY_FLAGS) -o resources/ankeec ankeec.c

install: build
	cp resources/ankeec /usr/local/bin/ankeec
	cp resources/sankee /usr/local/bin/sankee
	mkdir -p /usr/share/ankee/
	ln -fs $(PWD)/resources/JMdict_e.xml /usr/share/ankee/JMdict_e.xml
	ln -fs $(PWD)/ankeed /usr/local/bin/ankeed
	@echo '	Add'
	@echo '		if [ "$$ANKEEC" = "1" ]'
	@echo '		exec ankeec "$$(cat /tmp/ankeect)" "$$(cat /tmp/ankeecp)"'
	@echo '		end'
	@echo '	to ~/.config/fish/config.fish'

run: build
	./resources/ankeec "ab cd()\ /きのこ人間泣けば" "/tmp/mata"
