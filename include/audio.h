#ifndef AUDIO_H_INCLUDED
#define AUDIO_H_INCLUDED

/* Game audio, as events rather than scattered playback calls.
 *
 * Previously Mix_PlayChannel(-1, son, 0) appeared at seven call sites through
 * a play_click_sound() helper, and that was the entire sound design: the only
 * effect in the game was the menu click. Music was worse -- the settings
 * screen called Mix_VolumeMusic() four times, but nothing anywhere ever
 * called Mix_LoadMUS or Mix_PlayMusic, so assets/audio/music.mp3 was an
 * orphan file and the volume slider adjusted silence.
 *
 * Callers now name what happened and this module decides what it sounds like.
 */

#include "platform.h"

typedef enum {
  AUDIO_MENU_MOVE = 0,
  AUDIO_MENU_CLICK,
  AUDIO_JUMP,
  AUDIO_DAMAGE,
  AUDIO_GOAL,
  AUDIO_EVENT_COUNT
} audio_event;

/* Volume range exposed to the UI, matching the mixer's own scale. */
#define AUDIO_VOLUME_MAX 128
#define AUDIO_VOLUME_DEFAULT 64

/* Loads every effect and the music track. Safe to call when audio failed to
 * initialise: everything then becomes a no-op. */
void audio_init(void);
void audio_shutdown(void);

void audio_play(audio_event event);

/* Starts the looping background track. */
void audio_music_start(void);
/* Clamps to [0, AUDIO_VOLUME_MAX] and applies. Returns what was applied, so
 * the caller can keep its own copy in range too. */
int audio_music_volume(int volume);

#endif
