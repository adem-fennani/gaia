#include <stdlib.h>
#include <stdio.h>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>
#include "minimap.h"

void initmap(minimap* m)
{
	m->img = IMG_Load("Ressources/minimap.png");
	m->pos.x = 576;
	m->pos.y = 20;
	m->perso_img = IMG_Load("Ressources/fleche.png");
	m->perso_pos.x = 576;
	m->perso_pos.y = 94;
}

void init_background(backg* background)
{
	background->img = IMG_Load("Ressources/background_2.png");
	if (background->img == NULL)
	{
		printf("Erreur d'ouverture: %s\n", SDL_GetError());
		return;
	}
	background->pos.x = 0;
	background->pos.y = 0;	
	background->masque = IMG_Load("Ressources/masque.png");
	if (background->masque == NULL)
	{
		printf("Erreur d'ouverture: %s\n", SDL_GetError());
		return;
	}
	background->pos.x = 0;
	background->pos.y = 0;	
}

void init_perso(perso* perso)
{
	perso->img = IMG_Load("Ressources/perso.png");
	if (perso->img == NULL)
	{
		printf("Erreur d'ouverture: %s\n", SDL_GetError());
		return;
	}
	perso->pos.x = 0;
	perso->pos.y = 450;
	perso->pos.h = 144;
	perso->pos.w = 144;	
}

void afficher_background(backg background, SDL_Surface *screen)
{
	SDL_BlitSurface(background.img, NULL, screen, &background.pos);
}

void afficher_perso(perso perso, SDL_Surface *screen)
{
	SDL_BlitSurface(perso.img, NULL, screen, &perso.pos);
}

void afficherminimap (minimap m, SDL_Surface* screen)
{
	SDL_BlitSurface(m.img, NULL, screen, &m.pos) ;
	SDL_BlitSurface(m.perso_img, NULL, screen, &m.perso_pos) ;
}

SDL_Color GetPixel(SDL_Surface *backg, int x, int y)
{
    SDL_Color color;
    Uint32 col = 0;
    //Determine position
    char *pixelPosition = (char *)backg->pixels;
    //Offset by Y
    pixelPosition += (backg->pitch * y);
    //Offset by X
    pixelPosition += (backg->format->BytesPerPixel * x);
    //Copy pixel data
    memcpy(&col, pixelPosition, backg->format->BytesPerPixel);
    //Convert to color
    SDL_GetRGB(col, backg->format, &color.r, &color.g, &color.b);
    return (color);
}

int collisionPP(perso p, SDL_Surface *masque)
{
	SDL_Color test, couleurnoir = {0, 0, 0};
    SDL_Rect pos[8];
    pos[0].x = p.pos.x;
    pos[0].y = p.pos.y;
    pos[1].x = p.pos.x + p.pos.w / 2;
    pos[1].y = p.pos.y;
    pos[2].x = p.pos.x + p.pos.w;
    pos[2].y = p.pos.y;
    pos[3].x = p.pos.x;
    pos[3].y = p.pos.y + p.pos.h / 2;
    pos[4].x = p.pos.x;
    pos[4].y = p.pos.y + p.pos.h;
    pos[5].x = p.pos.x + p.pos.w / 2;
    pos[5].y = p.pos.y + p.pos.h;
    pos[6].x = p.pos.x + p.pos.w;
    pos[6].y = p.pos.y + p.pos.h;
    pos[7].x = p.pos.x + p.pos.w;
    pos[7].y = p.pos.y + p.pos.h / 2;
    int collision = 0, x, y;
    for (int i = 0; i < 8 && collision == 0; i++)
    {
        x = pos[i].x;
        y = pos[i].y;
        test = GetPixel(masque, x, y);
        if (test.r == 0 && test.g == 0 && test.b == 0)
            collision = 1;
    }
    return collision;
}

void MAJminimap(int pas, int direc, minimap* m, int redim)
{
	if(direc == 0)
	{
		m->perso_pos.x += pas * redim/100;
	}
	if(direc == 1)
	{
		m->perso_pos.x -= pas * redim/100;
	}
	if(direc == 2)
	{
		m->perso_pos.y -= pas * redim/100;
	}
	if(direc == 3)
	{
		m->perso_pos.y += pas * redim/100;
	}	
}

void liberer_backg(backg b)
{
	SDL_FreeSurface(b.img);
}

void liberer_perso(perso p)
{
	SDL_FreeSurface(p.img);
}

void liberer_minimap(minimap m)
{	
	SDL_FreeSurface(m.img) ; 
	SDL_FreeSurface(m.perso_img) ; 	
} 
