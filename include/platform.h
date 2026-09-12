#ifndef PLATFORM_H_INCLUDED
#define PLATFORM_H_INCLUDED

/* Platform layer: the single place that talks to SDL.
 *
 * Gameplay code calls plat_* instead of SDL_*, so the SDL 2.0 port in 0.5.0
 * can replace src/platform_sdl12.c without touching game logic.
 *
 * Worth knowing which SDL is actually underneath: on Ubuntu 24.04 and newer,
 * libsdl1.2-dev is sdl12-compat, which reimplements the SDL 1.2 API on top of
 * SDL2 (sdl-config reports 1.2.72; real SDL 1.2 ended at 1.2.15). So on a
 * current distro this already runs on SDL2 through a translation layer, and
 * the port removes that layer rather than changing platform.
 *
 * The types below are still raw SDL types. Making them opaque is 0.5.0 work --
 * doing it here would touch every struct in the tree for no behavioural gain.
 */

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_mixer.h>
#include <SDL/SDL_ttf.h>
#include <stdbool.h>

typedef SDL_Surface plat_surface;
typedef SDL_Rect plat_rect;
typedef SDL_Color plat_color;
typedef TTF_Font plat_font;
typedef Mix_Chunk plat_sound;
typedef Mix_Music plat_music;

/* --- lifecycle --------------------------------------------------------- */

/* Brings up video, image, audio and font subsystems and opens the window.
 * Returns false only when video is unusable; audio failure is non-fatal and
 * sets the mute flag that plat_audio_ok() reports. */
bool plat_init(int width, int height, const char *caption);
void plat_shutdown(void);
plat_surface *plat_screen(void);

/* False when Mix_OpenAudio failed, so callers can skip audio entirely rather
 * than issuing calls into a half-initialised mixer. */
bool plat_audio_ok(void);

/* --- images ------------------------------------------------------------ */

/* Never returns NULL. On failure it logs and returns a 64x64 magenta/black
 * checkerboard, so a missing asset renders as obvious placeholder art instead
 * of crashing. This is deliberate, not a workaround: NULL checks on the result
 * are dead code. scripts/verify_assets.py is what catches missing assets, at
 * build time, before this fallback can hide one. */
plat_surface *plat_image_load(const char *path);
void plat_image_free(plat_surface *surface);

/* Horizontal centre of the surface's lowest band of opaque pixels, in surface
 * coordinates. Used to anchor character frames by their feet: the sprite art
 * is tightly cropped and its frame width changes with the pose (99px idle,
 * 205px mid-stride), so there is no fixed offset that keeps a character
 * standing still while its animation plays. Returns w/2 if nothing is opaque
 * enough to measure. */
int plat_surface_foot_anchor(plat_surface *surface);

/* --- drawing ----------------------------------------------------------- */

void plat_blit(plat_surface *src, const plat_rect *src_rect,
               plat_surface *dst, plat_rect *dst_rect);
void plat_fill_rect(plat_surface *dst, const plat_rect *rect, Uint8 r, Uint8 g,
                    Uint8 b);
void plat_flip(plat_surface *screen);

/* --- text -------------------------------------------------------------- */

plat_font *plat_font_open(const char *path, int pt);
void plat_font_close(plat_font *font);
/* Renders, blits, and frees the text surface. */
void plat_text_draw(plat_surface *target, plat_font *font, const char *text,
                    int x, int y, plat_color color);

/* --- audio ------------------------------------------------------------- */

/* Returns NULL on failure, unlike plat_image_load: a missing sound is silent
 * and callers must check. */
plat_sound *plat_sound_load(const char *path);
void plat_sound_free(plat_sound *sound);
void plat_sound_volume(plat_sound *sound, int volume);
void plat_sound_play(plat_sound *sound);

plat_music *plat_music_load(const char *path);
/* Same, but silent when the file is simply absent -- for probing an optional
 * preferred format before falling back. */
plat_music *plat_music_load_optional(const char *path);
void plat_music_free(plat_music *music);
void plat_music_play(plat_music *music, int loops);
void plat_music_stop(void);
/* Clamps to the mixer's valid range. */
void plat_music_volume(int volume);
int plat_music_max_volume(void);

/* --- timing and input -------------------------------------------------- */

Uint32 plat_ticks(void);
void plat_delay(Uint32 ms);
bool plat_event_poll(SDL_Event *event);

#endif
