#include <stdio.h>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_mixer.h>
#include "utils.h"

static SDL_Surface* create_checkerboard_surface(void)
{
const int width = 64;
const int height = 64;
const int tile_size = 8;
SDL_Surface* fallback = SDL_CreateRGBSurface(SDL_SWSURFACE, width, height, 32, 0, 0, 0, 0);
if (fallback == NULL)
{
fprintf(stderr, "[load_image_safe] Unable to create fallback surface: %s\n", SDL_GetError());
return NULL;
}

Uint32 pink = SDL_MapRGB(fallback->format, 255, 0, 255);
Uint32 black = SDL_MapRGB(fallback->format, 0, 0, 0);
SDL_LockSurface(fallback);
for (int y = 0; y < height; ++y)
{
for (int x = 0; x < width; ++x)
{
Uint32* pixel = (Uint32*)((Uint8*)fallback->pixels + y * fallback->pitch + x * sizeof(Uint32));
*pixel = (((x / tile_size) + (y / tile_size)) % 2 == 0) ? pink : black;
}
}
SDL_UnlockSurface(fallback);
return fallback;
}

SDL_Surface* load_image_safe(const char* path)
{
SDL_Surface* surface = IMG_Load(path);
if (surface != NULL)
{
return surface;
}

fprintf(stderr, "[load_image_safe] Failed to load '%s': %s\n", path != NULL ? path : "(null)", SDL_GetError());
return create_checkerboard_surface();
}

Mix_Chunk* load_wav_safe(const char* path)
{
Mix_Chunk* chunk = Mix_LoadWAV(path);
if (chunk != NULL)
{
return chunk;
}

fprintf(stderr, "[load_wav_safe] Failed to load '%s': %s\n", path != NULL ? path : "(null)", Mix_GetError());
return NULL;
}
