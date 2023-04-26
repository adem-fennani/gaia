#include <stdlib.h>
#include <stdio.h>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>
#include "minimap.h"

int main()
{
	int boucle = 1;
	SDL_Event event;
	minimap m;
	backg b;
    perso p;
	SDL_Surface *screen;
	screen= SDL_SetVideoMode(1440,720, 32, SDL_HWSURFACE|SDL_DOUBLEBUF);
	init_background(&b);
	init_perso(&p);
	initmap(&m);
	while(boucle)
	{
		afficher_background(b, screen);
		afficher_perso(p, screen);
		afficherminimap (m, screen);
		while (SDL_PollEvent(&event))
        {
    		switch (event.type)
    		{
				case SDL_KEYDOWN:
				{
					switch(event.key.keysym.sym)
					{
						case SDLK_q:
							boucle = 0;
            				break;	
						case SDLK_RIGHT:
							if(p.pos.x < (1440 - p.pos.w))
							{
								p.pos.x += 40;
								MAJminimap(40, 0, &m, 20);
								printf("Collision: %d\n", collisionPP(p, b.masque));
							}
							else
							{
								p.pos.x = p.pos.x;
							}
							break;
						case SDLK_LEFT:
							if(p.pos.x >= 0)
							{
								p.pos.x -= 40;
								MAJminimap(40, 1, &m, 20);
								printf("Collision: %d\n", collisionPP(p, b.masque));
							}
							else
							{
								p.pos.x = p.pos.x;
							}
							break;
						case SDLK_UP:
							if(p.pos.y >= 0)
							{
								p.pos.y -= 40;
								MAJminimap(40, 2, &m, 20);
								printf("Collision: %d\n", collisionPP(p, b.masque));
							}
							else
							{
								p.pos.y = p.pos.y;
							}
							break;
						case SDLK_DOWN:
							if(p.pos.y < (720 - p.pos.w))
							{
								p.pos.y += 40;
								MAJminimap(40, 3, &m, 20);
								printf("Collision: %d\n", collisionPP(p, b.masque));
							}
							else
							{
								p.pos.y = p.pos.y;
							}
							break;
					}
					break;
				}
        		case SDL_QUIT:
				{
		  			boucle = 0;
            		break;
				}
			}
		}
		SDL_Flip(screen);
	}
	liberer_backg(b);
	liberer_perso(p);
	liberer_minimap(m);
	SDL_Quit();
	return 0;
}
