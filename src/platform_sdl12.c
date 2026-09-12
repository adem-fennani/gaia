/* SDL 1.2 backend for the platform layer. See include/platform.h.
 *
 * This file is the only place in the tree that should call SDL_*, IMG_*,
 * Mix_* or TTF_* directly. The SDL 2.0 port in 0.5.0 replaces this file.
 */

#include "../include/platform.h"
#include <stdio.h>

static plat_surface *g_screen = NULL;
static bool g_audio_ok = false;

/* --- lifecycle --------------------------------------------------------- */

bool plat_init(int width, int height, const char *caption) {
  /* SDL_VIDEO_CENTERED is read when the video subsystem starts, so it has to
   * be set before SDL_Init. It used to be set after, where it did nothing. */
  putenv("SDL_VIDEO_CENTERED=1");

  if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
    fprintf(stderr, "[plat_init] SDL_Init failed: %s\n", SDL_GetError());
    return false;
  }

  g_screen = SDL_SetVideoMode(width, height, 32, SDL_HWSURFACE | SDL_DOUBLEBUF);
  if (g_screen == NULL) {
    fprintf(stderr, "[plat_init] Can't set video mode: %s\n", SDL_GetError());
    return false;
  }
  SDL_WM_SetCaption(caption, NULL);

  IMG_Init(IMG_INIT_PNG);

  /* Mix_Init loads the decoder libraries. Without it, MP3 and OGG support
   * depends on the mixer lazily loading them, which is not guaranteed -- and
   * the project's only music track is an MP3. Missing codecs are not fatal:
   * the requested flags that came back tell us what is actually available. */
  Mix_Init(MIX_INIT_MP3 | MIX_INIT_OGG);

  /* Audio is optional: a machine with no working device should still play. */
  if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096) == -1) {
    fprintf(stderr, "[plat_init] Mix_OpenAudio failed, continuing muted: %s\n",
            Mix_GetError());
    g_audio_ok = false;
  } else {
    g_audio_ok = true;
  }

  if (TTF_Init() != 0) {
    fprintf(stderr, "[plat_init] TTF_Init failed: %s\n", TTF_GetError());
    return false;
  }
  return true;
}

void plat_shutdown(void) {
  if (g_audio_ok) {
    Mix_HaltMusic();
    Mix_CloseAudio();
    g_audio_ok = false;
  }
  /* Pairs with Mix_Init. Neither this nor Mix_CloseAudio existed anywhere in
   * the tree before, which is the "missing cleanup" half of issue #17. */
  Mix_Quit();
  TTF_Quit();
  IMG_Quit();
  /* g_screen belongs to SDL_SetVideoMode; SDL_Quit releases it. */
  g_screen = NULL;
  SDL_Quit();
}

plat_surface *plat_screen(void) { return g_screen; }

bool plat_audio_ok(void) { return g_audio_ok; }

/* --- images ------------------------------------------------------------ */

/* 64x64 magenta/black checkerboard, so a failed load is unmistakable on
 * screen rather than a crash or an invisible hole. */
static plat_surface *create_checkerboard_surface(void) {
  const int width = 64;
  const int height = 64;
  const int tile_size = 8;
  Uint32 pink;
  Uint32 black;
  int x;
  int y;
  plat_surface *fallback =
      SDL_CreateRGBSurface(SDL_SWSURFACE, width, height, 32, 0, 0, 0, 0);
  if (fallback == NULL) {
    fprintf(stderr, "[plat_image_load] Unable to create fallback surface: %s\n",
            SDL_GetError());
    return NULL;
  }

  pink = SDL_MapRGB(fallback->format, 255, 0, 255);
  black = SDL_MapRGB(fallback->format, 0, 0, 0);
  SDL_LockSurface(fallback);
  for (y = 0; y < height; ++y) {
    for (x = 0; x < width; ++x) {
      Uint32 *pixel = (Uint32 *)((Uint8 *)fallback->pixels +
                                 y * fallback->pitch + x * sizeof(Uint32));
      *pixel = (((x / tile_size) + (y / tile_size)) % 2 == 0) ? pink : black;
    }
  }
  SDL_UnlockSurface(fallback);
  return fallback;
}

plat_surface *plat_image_load(const char *path) {
  plat_surface *surface = IMG_Load(path);
  if (surface != NULL) {
    return surface;
  }
  fprintf(stderr, "[plat_image_load] Failed to load '%s': %s\n",
          path != NULL ? path : "(null)", SDL_GetError());
  return create_checkerboard_surface();
}

void plat_image_free(plat_surface *surface) {
  if (surface != NULL) {
    SDL_FreeSurface(surface);
  }
}

int plat_surface_foot_anchor(plat_surface *surface) {
  const Uint8 opaque = 200;
  /* Average over a short band rather than the single lowest row, so one stray
   * antialiased pixel cannot define the anchor. */
  const int band = 8;
  int y;
  int lowest = -1;
  long sum = 0;
  long count = 0;

  if (surface == NULL || surface->w <= 0 || surface->h <= 0) {
    return 0;
  }
  if (SDL_LockSurface(surface) != 0) {
    return surface->w / 2;
  }

  for (y = surface->h - 1; y >= 0; --y) {
    int x;
    int found = 0;
    for (x = 0; x < surface->w; ++x) {
      Uint8 r, g, b, a;
      Uint32 pixel = 0;
      Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch +
                 x * surface->format->BytesPerPixel;
      switch (surface->format->BytesPerPixel) {
      case 1:
        pixel = *p;
        break;
      case 2:
        pixel = *(Uint16 *)p;
        break;
      case 4:
        pixel = *(Uint32 *)p;
        break;
      default:
        /* 24-bit has no alpha channel to test, so treat it as opaque. */
        pixel = 0;
        break;
      }
      if (surface->format->BytesPerPixel == 3) {
        a = SDL_ALPHA_OPAQUE;
      } else {
        SDL_GetRGBA(pixel, surface->format, &r, &g, &b, &a);
      }
      if (a >= opaque) {
        found = 1;
        sum += x;
        count++;
      }
    }
    if (found && lowest < 0) {
      lowest = y;
    }
    if (lowest >= 0 && y <= lowest - band) {
      break;
    }
    if (lowest < 0) {
      /* Nothing opaque yet: discard what this row contributed. */
      sum = 0;
      count = 0;
    }
  }

  SDL_UnlockSurface(surface);
  if (count == 0) {
    return surface->w / 2;
  }
  return (int)(sum / count);
}

/* --- drawing ----------------------------------------------------------- */

void plat_blit(plat_surface *src, const plat_rect *src_rect, plat_surface *dst,
               plat_rect *dst_rect) {
  /* SDL_BlitSurface takes a non-const source rect but does not modify it;
   * the cast keeps the const promise at this layer's boundary. */
  if (src == NULL || dst == NULL) {
    return;
  }
  SDL_BlitSurface(src, (plat_rect *)src_rect, dst, dst_rect);
}

void plat_fill_rect(plat_surface *dst, const plat_rect *rect, Uint8 r, Uint8 g,
                    Uint8 b) {
  if (dst == NULL) {
    return;
  }
  SDL_FillRect(dst, (plat_rect *)rect, SDL_MapRGB(dst->format, r, g, b));
}

void plat_flip(plat_surface *screen) {
  if (screen != NULL) {
    SDL_Flip(screen);
  }
}

/* --- text -------------------------------------------------------------- */

plat_font *plat_font_open(const char *path, int pt) {
  plat_font *font = TTF_OpenFont(path, pt);
  if (font == NULL) {
    fprintf(stderr, "[plat_font_open] Failed to load '%s': %s\n",
            path != NULL ? path : "(null)", TTF_GetError());
  }
  return font;
}

void plat_font_close(plat_font *font) {
  if (font != NULL) {
    TTF_CloseFont(font);
  }
}

void plat_text_draw(plat_surface *target, plat_font *font, const char *text,
                    int x, int y, plat_color color) {
  plat_surface *surface;
  plat_rect pos;
  if (target == NULL || font == NULL || text == NULL) {
    return;
  }
  surface = TTF_RenderText_Solid(font, text, color);
  if (surface == NULL) {
    return;
  }
  pos.x = (Sint16)x;
  pos.y = (Sint16)y;
  SDL_BlitSurface(surface, NULL, target, &pos);
  /* Freed here every time. Rendering text per frame without freeing it is
   * exactly the leak afficherPerso() used to have. */
  SDL_FreeSurface(surface);
}

/* --- audio ------------------------------------------------------------- */

plat_sound *plat_sound_load(const char *path) {
  plat_sound *chunk;
  if (!g_audio_ok) {
    return NULL;
  }
  chunk = Mix_LoadWAV(path);
  if (chunk == NULL) {
    fprintf(stderr, "[plat_sound_load] Failed to load '%s': %s\n",
            path != NULL ? path : "(null)", Mix_GetError());
  }
  return chunk;
}

void plat_sound_free(plat_sound *sound) {
  if (sound != NULL) {
    Mix_FreeChunk(sound);
  }
}

void plat_sound_volume(plat_sound *sound, int volume) {
  if (sound != NULL) {
    Mix_VolumeChunk(sound, volume);
  }
}

void plat_sound_play(plat_sound *sound) {
  if (g_audio_ok && sound != NULL) {
    Mix_PlayChannel(-1, sound, 0);
  }
}

plat_music *plat_music_load(const char *path) {
  plat_music *music;
  if (!g_audio_ok) {
    return NULL;
  }
  music = Mix_LoadMUS(path);
  if (music == NULL) {
    fprintf(stderr, "[plat_music_load] Failed to load '%s': %s\n",
            path != NULL ? path : "(null)", Mix_GetError());
  }
  return music;
}

plat_music *plat_music_load_optional(const char *path) {
  if (!g_audio_ok) {
    return NULL;
  }
  return Mix_LoadMUS(path);
}

void plat_music_free(plat_music *music) {
  if (music != NULL) {
    Mix_HaltMusic();
    Mix_FreeMusic(music);
  }
}

void plat_music_play(plat_music *music, int loops) {
  if (g_audio_ok && music != NULL) {
    Mix_PlayMusic(music, loops);
  }
}

void plat_music_stop(void) {
  if (g_audio_ok) {
    Mix_HaltMusic();
  }
}

void plat_music_volume(int volume) {
  if (!g_audio_ok) {
    return;
  }
  if (volume < 0) {
    volume = 0;
  }
  if (volume > MIX_MAX_VOLUME) {
    volume = MIX_MAX_VOLUME;
  }
  Mix_VolumeMusic(volume);
}

int plat_music_max_volume(void) { return MIX_MAX_VOLUME; }

/* --- timing and input -------------------------------------------------- */

Uint32 plat_ticks(void) { return SDL_GetTicks(); }

void plat_delay(Uint32 ms) { SDL_Delay(ms); }

bool plat_event_poll(SDL_Event *event) { return SDL_PollEvent(event) != 0; }
