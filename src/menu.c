#include "../include/menu.h"
#include "../include/utils.h"
void init_background(image *background, char *nom) {
  background->img = load_image_safe(nom);
  background->pos.x = 0;
  background->pos.y = 0;
  background->pos.w = 1150;
  background->pos.h = 650;
}
void init_bouton(image *bouton1, char *nom1, image *bouton2, char *nom2,
                 image *bouton3, char *nom3) {
  bouton1->img = load_image_safe(nom1);
  bouton2->img = load_image_safe(nom2);
  bouton3->img = load_image_safe(nom3);
  bouton1->pos.x = 53;
  bouton2->pos.x = 53;
  bouton3->pos.x = 53;

  bouton1->pos.y = 275;
  bouton2->pos.y = 375;
  bouton3->pos.y = 475;
}
void init_volume_slayed(image *bouton_slide, char *nom) {
  bouton_slide->img = load_image_safe(nom);
  bouton_slide->pos.x = 515;
  bouton_slide->pos.y = 222;
}

void init_retour_bouton(image *retour, char *nom) {
  retour->img = load_image_safe(nom);
  retour->pos.x = 483;
  retour->pos.y = 391;
}

void affichier_imag(image p, SDL_Surface *screen) {
  SDL_BlitSurface(p.img, NULL, screen, &p.pos);
}

void librer(image p) {
  if (p.img != NULL) {
    SDL_FreeSurface(p.img);
  }
}
/*void init_text(SDL_Rect *pos_text)
{
pos_text->x = 480;
pos_text->y = 210;
}*/
