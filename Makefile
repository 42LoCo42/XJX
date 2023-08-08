SOURCES := $(wildcard *.cpp)
OUTPUT  := xjx
DEPS    := ncurses

CXXFLAGS := $(shell pkg-config --cflags $(DEPS)) -std=c++11 -Wall -Wextra
LDFLAGS  := $(shell pkg-config --libs   $(DEPS))

$(OUTPUT): $(SOURCES)

install: $(OUTPUT)
	mkdir -p $$out/bin
	cp $< $$out/bin
