CC = gcc
EXTRA_CFLAGS =
CFLAGS = -g -Wall -Wextra -Wshadow -Wpointer-arith $(EXTRA_CFLAGS) -I$(PWD)/include $(shell sdl-config --cflags)
LDFLAGS = -lSDL -lSDL_ttf -lSDL_image -lSDL_mixer

SRC = src/platform_sdl12.c src/menu.c src/perso.c \
      src/game.c src/hud.c src/state_menu.c src/state_settings.c \
      src/state_level1.c \
      src/state_victory.c src/collision.c src/enemy.c src/audio.c
OBJ = $(patsubst src/%.c, build/%.o, $(SRC))

all: pro

pro: $(OBJ)
	$(CC) $(OBJ) -o $@ $(LDFLAGS)

build/%.o: src/%.c | build
	$(CC) $(CFLAGS) -c $< -o $@

build:
	mkdir -p build

clean:
	rm -rf build/*.o pro pro_asan compile_commands.json build

# Warning-free is a project invariant; strict makes a regression fail the build.
strict:
	$(MAKE) clean
	$(MAKE) EXTRA_CFLAGS=-Werror pro

verify:
	python3 scripts/verify_assets.py

# AddressSanitizer + LeakSanitizer build. There is no valgrind dependency;
# gcc ships both. Deliberately not part of `check`: SDL_mixer 1.2's MP3
# decoder overruns its own buffer (see src/audio.c), so a run that loads the
# MP3 track aborts on a fault in third-party code. Supply an OGG track, or
# skip the music load, to exercise the rest.
pro_asan: $(SRC)
	$(CC) -g -O0 -fsanitize=address $(CFLAGS) $(SRC) -o $@ $(LDFLAGS) -lm

asan: pro_asan
	ASAN_OPTIONS=detect_leaks=1 SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy ./pro_asan

smoke: pro
	./scripts/smoke_boot.sh

# What CI runs, and what to run before a commit.
check: strict verify smoke

.PHONY: all clean strict verify smoke check asan
