# Changelog
All notable changes to this project will be documented in this file.
The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [0.4.0] - 2026-09-12
One complete end-to-end mission: menu to level to result and back, with real
collision, a patrolling enemy, audio, and CI.

### Added
- Continuous integration (`.github/workflows/build.yml`), over two genuinely
  different runtimes: Ubuntu 22.04 carries real SDL 1.2, while 24.04 and newer
  ship `sdl12-compat`, which reimplements the SDL 1.2 API on top of SDL2.
- `scripts/verify_assets.py`: resolves every `assets/...` literal in `src/*.c`
  case-sensitively, validates PNG chunk CRCs and IEND, checks dimensions
  against `assets/MANIFEST.tsv`, asserts each parallax layer is large enough
  for the source rect its rate implies, and lists assets nothing loads.
- `scripts/smoke_boot.sh`: headless boot that fails on any asset-load failure,
  which `load_image_safe()`'s placeholder fallback otherwise hides.
- `scripts/frame_probe.sh`: captures rendered frames headlessly, for
  byte-for-byte before/after comparison across refactors.
- `scripts/gen_placeholder_assets.py`: generates level, HUD, enemy and minimap
  art at the exact sizes the renderer requires, plus the four missing sound
  effects. Deterministic; writes `assets/MANIFEST.tsv`.
- `make strict`, `verify`, `smoke`, `check`, and `asan` targets. `-Werror` and
  `-Wpointer-arith` are now enforced.
- A platform layer (`include/platform.h`, `src/platform_sdl12.c`): the only
  place in the tree that calls SDL, so the 0.5.0 port is a backend swap.
- An explicit state machine (`include/game_state.h`, `src/game.c`,
  `src/state_*.c`) replacing the `while (done) switch (etat)` loop.
- Swept AABB collision with vertical response (`src/collision.c`). Obstacles
  can be stood on.
- Patrolling enemies with contact damage, invulnerability, knockback, and a
  stomp (`src/enemy.c`), plus a mission-failure result screen.
- A sound-event layer (`src/audio.c`): jump, damage, goal, menu move, menu
  click. Music now actually plays.
- A HUD module (`src/hud.c`) owning layout, the minimap, and the mission timer.
- Player 2 can run (`W`/`X`). `acc1` previously had no key bound at all.

### Changed
- Completing the mission now requires **both** players at the exit beacon.
- Physics runs on a fixed 1/60s timestep; one second of input produces the
  same displacement at 30, 60 and 144 fps.
- Movement is in px/s and px/s^2. It was raw milliseconds with `vitesse = 1`,
  so the linear term alone moved ~16px per frame and the quadratic term
  reached ~272px in a single frame — wider than the sprite.
- Animation advances on a 10fps accumulator instead of one frame per rendered
  frame.
- Score is distance travelled rather than a count of rendered frames.
- The player's collision box is a fixed 56x290 anchored at the feet, with
  sprites anchored on their own measured foot centre. Frames run 99-205px wide
  and 286-304px tall, so the previous box drifted as the animation played.
- Obstacles and the goal are bottom-aligned to the ground surface. They sat
  below it, buried in the dirt.
- Level 1's backdrop is built by extending `Niv1.png` to the full screen
  height rather than blitting an oversized source rect out of it.
- The menu polls input before drawing, so hover and selection are no longer a
  frame stale.
- `afficherPerso()` takes `const perso *` and draws only the character.
- `assets/audio/music.ogg` is preferred over the MP3 when present.

### Fixed
- **The background covered only half the screen.** `Niv1.png` is 2048x341 but
  `draw_level1_scene()` blitted a 1150x650 source rect from it, so SDL clipped
  to the 341 rows that existed and the bottom 309 rows of every frame kept
  whatever the previous frame left there. (#14)
- **There was no vertical collision at all.** The floor was a scalar captured
  by `ypos_jump()` at the instant the jump key was pressed — so it was
  "wherever you happened to be when you last jumped from standing" — and
  obstacles could not be landed on. (#15)
- **The music was never loaded.** The settings screen called
  `Mix_VolumeMusic()` four times, but nothing anywhere called `Mix_LoadMUS` or
  `Mix_PlayMusic`, so `assets/audio/music.mp3` was an orphan file and the
  volume slider adjusted silence. (#13)
- **The health bar was inverted, and had been since 2023.** The art is indexed
  by damage taken, not health: `barre_0.png` is five red hearts and
  `barre_5.png` is five empty ones, so full health selected the empty frame.
  Nothing caught it because `vie` was never decremented, which means the HUD
  always showed an empty bar at full health.
- **Every HUD element overlapped another.** Health bars were 455x104 at (0,0)
  and (575,0) with three lines of 40pt text drawn at y=10/34/58 — inside them,
  and overlapping each other — and the minimap was 780x130 at (576,20), 206px
  wider than the screen it was drawn on.
- **The volume slider desynced from the volume** on the first keypress: the
  knob moved a flat ±15px while the volume moved ±9 or ±10, and the knob
  started pinned at its minimum while the volume sat at its default of 64.
- Volume is clamped to [0, 128]. Both adjustment paths were unbounded.
- The frame limiter was inverted: `SDL_Delay(300 / dt)` shortened its delay as
  frames got slower, so a 1ms frame slept 300ms, the next `dt` measured ~301
  and slept 0ms, and the loop oscillated instead of converging.
- Asset paths in `src/minimap.c` spelled `ressources` where the directory is
  `Ressources`, so `init_temps()` failed silently at every boot and the
  mission timer could never render. (#16)
- Entering level 1 after leaving it with `ESC` resumed mid-mission; state now
  resets on every entry. (#10)
- `UP` and `O` never actually jumped — they set `up = 1` without any upward
  velocity, starting a fall from midair. Jumping now requires being grounded.
- `DOWN` and `L` teleported the player 20px downward per key repeat.
- Mouse motion read `event.button.x/y`, which worked only because the two
  structs share offsets in the union.
- `putenv("SDL_VIDEO_CENTERED=1")` ran after `SDL_Init`, where it did nothing.
- `Mix_Init` was never called, so MP3 support depended on the mixer lazily
  loading a decoder. `Mix_Quit`, `Mix_CloseAudio` and `Mix_HaltMusic` did not
  exist anywhere in the tree. (#17)
- Audio calls no longer run against a half-initialised mixer when
  `Mix_OpenAudio` fails.
- Four memory leaks. The worst leaked two SDL surfaces per frame, forever:
  `afficherPerso()` took `perso` by value and assigned a rendered text surface
  to `p.score` on the caller's *copy*, so its only pointer died on return while
  `cleanup_game()` faithfully freed the always-NULL originals. Also fixed: the
  unfreed menu background, the minimap surfaces whose `liberer_minimap()` was
  never called, and the timer font. Verified clean under LeakSanitizer.
- Knockback could wedge a player inside an obstacle: it wrote position
  directly, and the X resolver then pushed an already-overlapping body out
  along its direction of motion, sending it through to the far face.

### Removed
- `src/minimap.c` (448 lines), `include/minimap.h` (116), and the 22-file
  `minimap/` prototype directory. This included a complete, unreachable
  tic-tac-toe game whose entry point would have called
  `SDL_SetVideoMode(600, 600)` and resized the window out from under its
  caller, and a timer apparatus superseded by `src/hud.c`.
- `src/utils.c` and `include/utils.h`, absorbed into the platform layer.
- `src/main_menu.c` (808 lines), dissolved into the state machine.
- `perso.score` and `perso.scor`, vestigial after the HUD took over scoring.
- A duplicate `TTF_Init()` and a stray `TTF_Quit()` that would have
  double-quit against shutdown.
- The last two non-English comments, in `include/menu.h`. The v0.3.0 sweep
  covered `src/` but not `include/`.

### Notes
- The PNG assets were never corrupt. Earlier releases claimed `libpng` aborted
  on them; all 89 pass chunk-CRC and IEND validation and a headless boot emits
  no libpng warnings. `Niv1.png` does carry a 2px near-white column on its left
  edge, an export artifact now trimmed by the generator, which may be what was
  originally seen.
- SDL_mixer 1.2's MP3 decoder reads past its own buffer. See the README; the
  fix is to ship the music as OGG.

## [0.3.0] - 2026-05-25
### Changed
- Removed all non-English comments across the `src/` directory.
- Enabled strict compiler warnings (`-Wall`, `-Wextra`, `-Wshadow`) in Makefile.
- Excluded Doxygen build folders and binaries via `.gitignore`.
### Fixed
- Addressed multiple IDE/Qodana static analysis security and stability warnings.
- Guaranteed all dynamically allocated bounds (`malloc`, `SDL_CreateRGBSurface`, `TTF_OpenFont`) have rigorous `NULL` check limits before dereferencing variables in structures like `perso` and `backgame`.
- Avoided missing event fallthroughs by adding explicit `default: break` conditions for `SDL_PollEvent` keystrokes logic.
- Mitigated unhandled unused variables and scope shadowing throughout `src/main_menu.c` and `src/minimap.c`.
- Hardened all deallocation cleanup sweeps (`SDL_FreeSurface`, `Mix_FreeMusic`, `TTF_CloseFont`) checking actively for unused null states first.

## [0.2.0] - 2026-05-13
### Fixed
- Fixed function name collision in `minimap.h` and `menu.h` for `init_background`.
- Fixed segmentation faults and heap corruption caused by incorrect memory allocation for `perso` struct (`barre` array).
- Fixed floating point exception (division by zero) in calculating `dt` for frame delay.
- Reduced audio noise and clipping by optimizing `Mix_OpenAudio` and `Mix_Volume` parameters.
- Reordered core SDL initialization (`SDL_Init` moved to the very top) to prevent start-up unresponsiveness.
### Added
- Added `IMG_Init` for explicit SDL_image PNG loading capability.
- Added defensive `NULL` pointer checks during surface rendering to prevent crashes.
- Implemented temporary dummy SDL surfaces to bypass corrupted PNG assets during `IMG_Load`, allowing the game to boot.

## [0.1.0] - 2023-05
### Added
- Integrated initial complete game source code.
- Combined main menu, settings menu, and gameplay structure into a single repository.
- Baseline source files (`main_menu.c`, `menu.c`, `minimap.c`, `perso.c`) containing original logic structure.
