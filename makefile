CC = gcc
EXTRA_CFLAGS =
CFLAGS = -g -Wall -Wextra -Wshadow -Wpointer-arith $(EXTRA_CFLAGS) -I$(PWD)/include $(shell sdl-config --cflags)
LDFLAGS = -lSDL -lSDL_ttf -lSDL_image -lSDL_mixer

SRC = src/platform_sdl12.c src/menu.c src/perso.c src/minimap.c \
      src/game.c src/state_menu.c src/state_settings.c src/state_level1.c \
      src/state_victory.c
OBJ = $(patsubst src/%.c, build/%.o, $(SRC))

all: pro

pro: $(OBJ)
	$(CC) $(OBJ) -o $@ $(LDFLAGS)

build/%.o: src/%.c | build
	$(CC) $(CFLAGS) -c $< -o $@

build:
	mkdir -p build

clean:
	rm -rf build/*.o pro compile_commands.json build

# Warning-free is a project invariant; strict makes a regression fail the build.
strict:
	$(MAKE) clean
	$(MAKE) EXTRA_CFLAGS=-Werror pro

verify:
	python3 scripts/verify_assets.py

smoke: pro
	./scripts/smoke_boot.sh

# What CI runs, and what to run before a commit.
check: strict verify smoke

.PHONY: all clean strict verify smoke check
