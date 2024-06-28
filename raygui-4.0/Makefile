PROJECT = $(notdir $(CURDIR))
OUTPUT = ../target/raylib/raygui.so

all: release

SRC = raygui.h
CC = gcc
FLAGS = -shared -fpic -DRAYGUI_IMPLEMENTATION -L"../target/raylib/" -I"../raylib-4.5.0/src/" -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
# Non production ready flags (as of 2021-09-01), https://github.com/google/sanitizers/issues/1324: -fsanitize=pointer-compare -fsanitize=pointer-subtract
#debug: FLAGS += -fsanitize=address -fsanitize=leak -fno-omit-frame-pointer -fsanitize=undefined -fsanitize=bounds-strict -fsanitize=float-divide-by-zero -fsanitize=float-cast-overflow $(shell export ASAN_OPTIONS=strict_string_checks=1:detect_stack_use_after_return=1:check_initialization_order=1:strict_init_order=1:detect_invalid_pointer_pairs=2) -fanalyzer -g

release: $(SRC)
	$(CC) -o $(OUTPUT) $(SRC) $(FLAGS)

