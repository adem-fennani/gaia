CC = gcc
CFLAGS = -g -Iinclude
LDFLAGS = -lSDL -lSDL_ttf -lSDL_image -lSDL_mixer

SRC = src/menu.c src/main_menu.c src/perso.c src/minimap.c src/utils.c
OBJ = $(SRC:.c=.o)

pro: $(OBJ)
	$(CC) $(OBJ) -o $@ $(LDFLAGS)

src/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) pro
