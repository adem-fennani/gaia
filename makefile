CC = gcc
CFLAGS = -g -I$(PWD)/include $(shell sdl-config --cflags)
LDFLAGS = -lSDL -lSDL_ttf -lSDL_image -lSDL_mixer

SRC = src/menu.c src/main_menu.c src/perso.c src/minimap.c src/utils.c
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
