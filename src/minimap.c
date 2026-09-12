/**
 * @file minimap.c
 * @brief Testing Program.
 * @author Adem FENNANI
 * @version 4.0
 * @date May 09, 2023
 *
 * Testing program for background scrollilng
 *
 */
#include "../include/minimap.h"
#include "../include/platform.h"
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>

/**
 * @brief Initialize the minimap
 * @param m pointer to minimap
 * @return Nothing
 */
void initmap(minimap *m) {
  /* The 260x44 minimap and its marker are generated from level 1's real
   * geometry by scripts/gen_placeholder_assets.py. The previous art was
   * assets/img/minimap.png at 780x130 -- 206px wider than the screen it was
   * drawn on, and cave-themed while the level is volcanic.
   *
   * Positions are not set here: src/hud.c owns HUD layout and places both the
   * panel and the per-player markers every frame. */
  m->img = plat_image_load("assets/img/hud/minimap.png");
  /* perso_img/perso_pos stay unset: the two player markers live on
   * game_assets, since this struct has only one slot and there are two
   * players. Both fields go away with the rest of the dead minimap code. */
  m->perso_img = NULL;
  memset(&m->pos, 0, sizeof(m->pos));
  memset(&m->perso_pos, 0, sizeof(m->perso_pos));
}

/**
 * @brief Initialize the background
 * @param bachg pointer to background
 * @return Nothing
 */
void init_background_minimap(backg *bg) {
  bg->img = plat_image_load("assets/img/Ressources/background_2.png");
  if (bg->img == NULL) {
    printf("Error opening: %s\n", SDL_GetError());
    return;
  }
  bg->pos.x = 0;
  bg->pos.y = 0;
  bg->masque = plat_image_load("assets/img/Ressources/masque.png");
  if (bg->masque == NULL) {
    printf("Error opening: %s\n", SDL_GetError());
    return;
  }
  bg->pos.x = 0;
  bg->pos.y = 0;
}

/**
 * @brief Initialize the character
 * @param perso pointer to character
 * @return Nothing
 * @deprecated Legacy function - use perso.h functions instead
 */
/*
void init_perso(perso* perso)
{
        perso->img = plat_image_load("assets/img/Ressources/perso.png");
        if (perso->img == NULL)
        {
                printf("Error opening: %s\n", SDL_GetError());
                return;
        }
        perso->pos.x = 0;
        perso->pos.y = 450;
        perso->pos.h = 144;
        perso->pos.w = 144;
}
*/

void afficher_background(backg background, SDL_Surface *screen) {
  plat_blit(background.img, NULL, screen, &background.pos);
}

/*
void afficher_perso(perso perso, SDL_Surface* screen)
{
        plat_blit(perso.img, NULL, screen, &perso.pos);
}
*/

void afficherminimap(minimap m, SDL_Surface *screen) {
  plat_blit(m.img, NULL, screen, &m.pos);
  plat_blit(m.perso_img, NULL, screen, &m.perso_pos);
}

SDL_Color GetPixel(SDL_Surface *surf, int x, int y) {
  SDL_Color color;
  Uint32 col = 0;
  // Determine position
  char *pixelPosition = (char *)surf->pixels;
  // Offset by Y
  pixelPosition += (surf->pitch * y);
  // Offset by X
  pixelPosition += (surf->format->BytesPerPixel * x);
  // Copy pixel data
  memcpy(&col, pixelPosition, surf->format->BytesPerPixel);
  // Convert to color
  SDL_GetRGB(col, surf->format, &color.r, &color.g, &color.b);
  return (color);
}

/*
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
*/

void MAJminimap(int pas, int direc, minimap *m, int redim) {
  if (direc == 0) {
    m->perso_pos.x += pas * redim / 100;
  }
  if (direc == 1) {
    m->perso_pos.x -= pas * redim / 100;
  }
  if (direc == 2) {
    m->perso_pos.y -= pas * redim / 100;
  }
  if (direc == 3) {
    m->perso_pos.y += pas * redim / 100;
  }
}

void minuteur(int *deb) {
  if (plat_ticks() - *deb >= 1000) {
    *deb = plat_ticks();
  }
}

int charger_font(text *t, char *fich) {
  if (TTF_Init() == -1) {
    printf("Error initializing TTF_Init : %s\n", TTF_GetError());
    return -1;
  }
  t->police = plat_font_open(fich, 40);
  if (t->police == NULL) {
    printf("Unable to load Font: %s\n", SDL_GetError());
    return (-1);
  }
  return (0);
}

int init_text_temps(text *t) {
  int testload;
  t->couleur.r = 0;
  t->couleur.g = 0;
  t->couleur.b = 0;
  strcpy(t->txt, "");
  t->pos.x = 20;
  t->pos.y = 20;
  testload = charger_font(t, "assets/img/Ressources/LemonMilk.otf");
  t->surf = NULL;
  return testload;
}

int init_temps(temps *t) {
  int test;
  t->deb = plat_ticks();
  t->min = 0;
  t->sec = 0;
  test = init_text_temps(&t->text);
  return test;
}

void MAJ_temps(temps *t) {
  int ts;
  minuteur(&t->deb);
  ts = t->deb / 1000;
  t->min = ts / 60;
  t->sec = ts % 60;
  if (t->min < 10 &&
      t->sec < 10) // Ensure the time is formatted with leading zeros
    sprintf(t->text.txt, "Temps: 0%d:0%d", t->min, t->sec);
  else if (t->min < 10 && t->sec >= 10)
    sprintf(t->text.txt, "Temps: 0%d:%d", t->min, t->sec);
  else if (t->min > 10 && t->sec < 10)
    sprintf(t->text.txt, "Temps: %d:0%d", t->min, t->sec);
  t->text.surf =
      TTF_RenderText_Solid(t->text.police, t->text.txt, t->text.couleur);
}

void afficher_temps(temps t, SDL_Surface *screen) {
  plat_blit(t.text.surf, NULL, screen, &(t.text.pos));
}

void liberer_text(text t) {
  if (t.police != NULL) {
    plat_font_close(t.police);
  }
  TTF_Quit();
}

void liberer_backg(backg b) {
  if (b.img != NULL) {
    plat_image_free(b.img);
  }
  if (b.masque != NULL) {
    plat_image_free(b.masque);
  }
}

/*
void liberer_perso(perso p)
{
        plat_image_free(p.img);
}
*/

void liberer_minimap(minimap m) {
  if (m.img != NULL) {
    plat_image_free(m.img);
  }
  if (m.perso_img != NULL) {
    plat_image_free(m.perso_img);
  }
}

// Tâche en Blanc

void quitgame(int *q, tic c) {
  Uint8 *keystate = SDL_GetKeyState(NULL);
  if (keystate[SDLK_ESCAPE])
    (*q) = 1;
  int count = 0;
  for (int i = 0; i < 3; i++)
    for (int j = 0; j < 3; j++)
      if (c.i[i][j] != -1)
        count++;
  if (count == 9)
    (*q) = 1;
}

void init(tic *c) {
  c->table = plat_image_load("assets/img/Ressources/xo.png");
  c->t[0] = plat_image_load("assets/img/Ressources/X.png");
  c->t[1] = plat_image_load("assets/img/Ressources/O.png");
  c->bg = plat_image_load("assets/img/Ressources/white.jpeg");
  if (c->t[1] == NULL)
    printf("Affichage réussi\n");
  for (int i = 0; i < 3; i++)
    for (int j = 0; j < 3; j++) {
      c->i[i][j] = -1;
      c->r[i][j].x = 217 * i + 40;
      c->r[i][j].y = 217 * j + 40;
      c->turn = rand() % 2;
      c->choixx = 0;
      c->choixy = 0;
    }
}

void show(tic c, SDL_Surface *screen) {
  plat_blit(c.bg, NULL, screen, NULL);
  plat_blit(c.table, NULL, screen, NULL);
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
      if (c.i[i][j] != -1) {
        plat_blit(c.t[c.i[i][j]], NULL, screen, &c.r[i][j]);
      }
      if ((c.choixx == i) && (c.choixy == j)) {
        plat_blit(c.t[c.turn], NULL, screen, &c.r[i][j]);
      }
    }
  }
  plat_flip(screen);
}

void turn_played(tic *c) {
  c->turn++;
  if (c->turn == 2)
    c->turn = 0;
}

void change(tic *c) {
  plat_delay(100);
  Uint8 *keystate = SDL_GetKeyState(NULL);
  if (keystate[SDLK_RIGHT])
    if (c->choixx < 2)
      c->choixx++;
  if (keystate[SDLK_LEFT])
    if (c->choixx > 0)
      c->choixx--;
  if (keystate[SDLK_DOWN])
    if (c->choixy < 2)
      c->choixy++;
  if (keystate[SDLK_UP])
    if (c->choixy > 0)
      c->choixy--;
  if (keystate[SDLK_RETURN])
    if (c->i[c->choixx][c->choixy] == -1) {
      c->i[c->choixx][c->choixy] = c->turn;
      turn_played(&(*c));
    }
}

int winner_is(tic c) {
  for (int i = 0; i < 2; i++) {
    for (int j = 0; j < 3; j++) {
      if ((c.i[j][0] == i) && (c.i[j][1] == i) && (c.i[j][2] == i))
        return i;
      if ((c.i[0][j] == i) && (c.i[1][j] == i) && (c.i[2][j] == i))
        return i;
    }
    if ((c.i[0][0] == i) && (c.i[1][1] == i) && (c.i[2][2] == i))
      return i;
    if ((c.i[0][2] == i) && (c.i[1][1] == i) && (c.i[2][0] == i))
      return i;
  }
  return -1;
}

void ia2(tic *c) {
  int count = 0;
  if (c->turn == 1) {
    plat_delay(500);
    for (int i = 0; i < 2; i++) {
      for (int j = 0; j < 3; j++) {
        count = 0;
        for (int k = 0; k < 3; k++)
          if (c->i[j][k] == i)
            count++;
        if (count == 2)
          for (int k = 0; k < 3; k++)
            if (c->i[j][k] == -1) {
              c->i[j][k] = 1;
              turn_played(&(*c));
              return;
            }
        count = 0;
        for (int k = 0; k < 3; k++)
          if (c->i[k][j] == i)
            count++;
        if (count == 2)
          for (int k = 0; k < 3; k++)
            if (c->i[k][j] == -1) {
              c->i[k][j] = 1;
              turn_played(&(*c));
              return;
            }
        count = 0;
      }
      for (int k = 0; k < 3; k++)
        if (c->i[k][k] == i)
          count++;
      if (count == 2)
        for (int k = 0; k < 3; k++)
          if (c->i[k][k] == -1) {
            c->i[k][k] = 1;
            turn_played(&(*c));
            return;
          }
      count = 0;
      if (c->i[0][2] == i)
        count++;
      if (c->i[1][1] == i)
        count++;
      if (c->i[2][0] == i)
        count++;
      if (count == 2) {
        if (c->i[0][2] == -1) {
          c->i[0][2] = 1;
          turn_played(&(*c));
          return;
        }
        if (c->i[2][0] == -1) {
          c->i[2][0] = 1;
          turn_played(&(*c));
          return;
        }
        if (c->i[1][1] == -1) {
          c->i[1][1] = 1;
          turn_played(&(*c));
          return;
        }
      }
    }
    for (int i = 0; i < 3; i++)
      for (int j = 0; j < 3; j++)
        if (c->i[i][j] == -1) {
          c->i[i][j] = 1;
          turn_played(&(*c));
          return;
        }
  }
}
int game(SDL_Surface *screen) {
  int q = 0;
  tic c;
  init(&c);
  screen = SDL_SetVideoMode(600, 600, 32, SDL_SWSURFACE);
  while (q == 0) {
    SDL_PumpEvents();
    quitgame(&q, c);
    if (c.turn == 0)
      change(&c);
    ia2(&c);
    show(c, screen);
    if (winner_is(c) != -1) {
      if (winner_is(c) == 1)
        plat_flip(screen);
      plat_delay(1000);
      return winner_is(c);
    }
  }
  plat_delay(1000);
  return -1;
}
