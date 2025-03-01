PROJECT = $(notdir $(CURDIR))
DEBUG = target/debug/$(PROJECT)
RELEASE = target/release/$(PROJECT)

all: mkdir debug

mkdir:
	mkdir -p ./target/debug
	mkdir -p ./target/release

SRC = $(wildcard src/*.c)
CC = gcc
FLAGS = -Wall -pipe
FLAGS += -I"$(CURDIR)/raylib-5.0/src/" -L"$(CURDIR)/target/raylib/" -I"$(CURDIR)/raygui-4.0/" -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
debug: OUTPUT = $(DEBUG)
debug: FLAGS += -g
# Non production ready flags (as of 2021-09-01), https://github.com/google/sanitizers/issues/1324: -fsanitize=pointer-compare -fsanitize=pointer-subtract
#debug: FLAGS += -fsanitize=address -fsanitize=leak -fno-omit-frame-pointer -fsanitize=undefined -fsanitize=bounds-strict -fsanitize=float-divide-by-zero -fsanitize=float-cast-overflow $(shell export ASAN_OPTIONS=strict_string_checks=1:detect_stack_use_after_return=1:check_initialization_order=1:strict_init_order=1:detect_invalid_pointer_pairs=2) -fanalyzer -g
debug: executable

release: OUTPUT = $(RELEASE)
release: FLAGS += -O2
release: executable

executable: $(SRC)
	$(CC) $(SRC) -o $(OUTPUT) $(FLAGS) 

.PHONY: run clean install uninstall

# write "make run a="..." for commandline arguments"
run:
	./$(DEBUG) $(a)

clean:
	rm -f $(DEBUG) $(RELEASE)

# installs from release folder only
install:
	ln -s $(CURDIR)/$(RELEASE) ~/.local/bin/

uninstall:
	rm -f ~/.local/bin/$(PROJECT)
