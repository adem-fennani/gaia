pro:menu.o main_menu.o perso.o minimap.o utils.o
	gcc menu.c main_menu.c perso.c minimap.c utils.c -o pro -lSDL -lSDL_ttf -g -lSDL_image -lSDL_mixer
menu.o:menu.c
	gcc -c menu.c -g
main_menu.o:main_menu.c
	gcc -c main_menu.c -g
	
perso.o:perso.c
	gcc -c perso.c
	
minimap.o:minimap.c
	gcc -c minimap.c -g

utils.o:utils.c
	gcc -c utils.c -g
