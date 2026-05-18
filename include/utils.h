#ifndef UTILS_H_INCLUDED
#define UTILS_H_INCLUDED

#include <SDL/SDL.h>
#include <SDL/SDL_mixer.h>

SDL_Surface *load_image_safe(const char *path);
Mix_Chunk *load_wav_safe(const char *path);

#endif
