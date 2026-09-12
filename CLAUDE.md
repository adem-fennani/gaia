# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with
code in this repository.

## Project

Gaia is a 2D local-co-op platformer written in C on **SDL 1.2** (`SDL_image`,
`SDL_mixer`, `SDL_ttf`). It builds to a single executable, `pro`, which must be
run from the repository root because every asset path is relative
(`assets/img/...`).

**What "SDL 1.2" actually means here.** On Ubuntu 24.04 and newer,
`libsdl1.2-dev` is `sdl12-compat`, a reimplementation of the SDL 1.2 API on top
of SDL2. Locally `sdl-config --version` reports 1.2.72 (real SDL 1.2 ended at
1.2.15) and `./pro` links `libSDL2` and `libSDL3`. Compatibility quirks on
modern Linux are most likely emulation gaps in that shim, not legacy SDL 1.2
behaviour. Ubuntu 22.04 still has the real library, which is why CI covers both.

## Build & Run

```bash
make            # or `make all` / `make pro`
./pro           # run from repo root; assets are loaded via relative paths
make clean
```

Dependencies (Debian/Ubuntu): `build-essential libsdl1.2-dev
libsdl-image1.2-dev libsdl-mixer1.2-dev libsdl-ttf2.0-dev`. Include flags come
from `sdl-config --cflags`. Regenerating placeholder art also needs Pillow; the
build does not.

The makefile has a hardcoded source list (`SRC`); a new `src/*.c` file is not
picked up until it is added there. Warnings are strict (`-Wall -Wextra
-Wshadow -Wpointer-arith`) and the tree builds warning-free — keep it that way.

## Verification

There is **no unit-test suite**. These are the safety net, and `make check` is
what CI runs:

```bash
make check      # strict build + asset verification + smoke boot
make strict     # -Werror
make verify     # asset paths resolve case-sensitively; PNGs valid; sizes match
                # MANIFEST.tsv; parallax layers big enough; orphans listed
make smoke      # headless boot; fails on any asset-load failure
make asan       # AddressSanitizer + LeakSanitizer
```

Two things worth knowing before changing anything:

- **`scripts/frame_probe.sh <dir>`** captures level-1 frames headlessly by
  temporarily patching `src/game.c` and restoring it. For any change that
  should *not* alter rendering, capture before and after and require the frames
  to be byte-identical. This is how the platform-wrapper and state-machine
  refactors were validated.
- **`src/collision.c` and `src/enemy.c` need no window or event loop.** Compile
  them against a small driver and assert on box positions and health directly.
  This found both of the bugs fixed in the enemy work; prefer it over reasoning
  about physics from the rendered output.

`make asan` is deliberately outside `make check`: SDL_mixer 1.2's MP3 decoder
reads past its own buffer, so a run that loads `music.mp3` aborts in
third-party code. `src/audio.c` prefers `assets/audio/music.ogg` when present,
which sidesteps it.

## Architecture

### Platform layer

`include/platform.h` is the **only** SDL surface in the tree, implemented by
`src/platform_sdl12.c`. Game code calls `plat_*`, never `SDL_*`/`IMG_*`/`Mix_*`/
`TTF_*`. The SDL2 port replaces that one file. If you find yourself adding an
SDL call anywhere else, add a wrapper instead.

`plat_image_load()` **never returns NULL** — on failure it logs and returns a
64×64 magenta/black checkerboard, so a missing asset renders as obvious
placeholder art. This is deliberate, which makes `NULL` checks on the result
dead code. `make verify` is what catches missing assets, at build time, before
the fallback can hide one. `plat_sound_load()` *does* return NULL; callers must
check.

### State machine

`src/game.c` owns one `game_ctx` and drives transitions. Each state is a module
exporting a `game_state_handlers` table (`on_enter`, `on_event`, `on_update`,
`on_draw`, `on_exit`; any may be NULL):

| State | File |
|---|---|
| `ST_MENU` | `src/state_menu.c` |
| `ST_LEVEL1` | `src/state_level1.c` |
| `ST_SETTINGS` | `src/state_settings.c` |
| `ST_VICTORY` | `src/state_victory.c` (also the failure screen) |

Transitions go through `game_request(ctx, ST_X)` and are applied **between**
frames, so a handler can request one without its remaining work being skipped
and `on_exit`/`on_enter` always run in pairs. Never assign `ctx->state`
directly. `SDL_QUIT` is handled once in the loop, not per state.

Lifecycle is fixed: `init_engine()` → `load_game_resources()` →
`run_game_loop()` → `cleanup_game()`. **All** resources for **all** states are
still loaded up front and freed in one place, so adding a state or an asset
means editing both functions.

### World space, camera, and drawing

Entity positions are **world** coordinates. Nothing is translated until draw
time, where `camera.x` is subtracted. Keep this split: gameplay and collision
work in world space, only the draw helpers know about the camera.

- `LEVEL1_W` (2300) and `LEVEL1_GROUND_STRIP_Y` (515) are in `include/perso.h`.
- `update_camera()` centres on the midpoint of both players, clamped to
  `[0, LEVEL1_W - SCREEN_W]`.
- Parallax: two layers at rates 1/2 and 3/4, generated at exactly the width
  those rates require. `make verify` asserts each layer is large enough for the
  source rect its rate implies — this is what stops a repeat of the bug where a
  2048×341 backdrop was blitted with a 1150×650 source rect, leaving the bottom
  309 rows of every frame unwritten.
- Level 1 geometry is hardcoded in `load_game_resources()`, bottom-aligned to
  the ground surface.

### Collision and movement

`src/collision.c` resolves a swept AABB, **X then Y**, against the obstacles and
the ground plane, setting `on_ground`. Resolving one axis at a time against the
pre-move position on that axis is what makes landing on a solid unambiguous. If
a body was already overlapping before the move, it exits via the *nearer* face,
not along its motion — writing a position directly and letting the resolver sort
it out is how knockback used to shove players through to the far side of a crate.

Positions are **doubles** (`perso.wx`/`wy`), with `pos_background` derived from
them by `perso_sync_rect()` for drawing and camera use. Truncating to
`SDL_Rect`'s `Sint16` every frame is what used to bend the jump arc away from
the parabola its own velocities implied.

Physics runs on a **fixed 1/60s timestep** from an accumulator, with both the
accumulator and `dt` capped. All movement constants are px/s or px/s² — see
`include/perso.h`. Never reintroduce per-frame deltas.

The player's collision box is a fixed `PLAYER_BOX_W`×`PLAYER_BOX_H` (56×290)
anchored at the feet, deliberately independent of the sprite: frames run
99–205px wide and 286–304px tall as the pose changes. Sprites are bottom-aligned
to the box and anchored horizontally on their own foot centre, measured at load
time by `plat_surface_foot_anchor()` — the art is tightly cropped with no
registration point, so there is no fixed offset that keeps a character still
while it walks.

### Two players

`p` and `p1` are separate `perso` instances from `initPerso()` / `initPerso1()`,
two near-duplicate functions differing only in spawn x and health-bar directory.
**Changes to player init almost always need applying to both.** Controls:

| | Player 1 | Player 2 |
|---|---|---|
| Move | ← → | K / M |
| Jump | Space / ↑ | J / O |
| Run | A / D | W / X |

Completing the mission requires **both** players in the goal. Either player's
health reaching zero fails it.

`perso` still mixes physics and render state, and `image[2][9]` is declared where
only `[2][7]` is filled; the load loops in `src/perso.c` and the free loops in
`cleanup_game()` must stay in sync with that count, as must `barre`'s six frames.

### HUD

`src/hud.c` owns HUD layout, with the geometry as constants in `include/hud.h`.
Elements previously positioned themselves independently and all collided. The
HUD owns the health bars and scores; `afficherPerso()` draws only the character.

**The health bar art is indexed by damage, not health**: `barre_0` is five red
hearts (full) and `barre_5` is empty, so the mapping is
`barre[HUD_VIE_MAX - vie]`. Getting this backwards is not hypothetical — it
shipped that way and went unnoticed because `vie` was never decremented.

### Audio

`src/audio.c` exposes events (`AUDIO_JUMP`, `AUDIO_DAMAGE`, `AUDIO_GOAL`,
`AUDIO_MENU_MOVE`, `AUDIO_MENU_CLICK`); callers name what happened and the
module decides what it sounds like. Do not call `plat_sound_*` from game code.
Volume goes through `audio_music_volume()`, which clamps and returns what it
applied. The settings slider's knob position is *derived* from the volume, never
nudged alongside it.

### Assets

`scripts/gen_placeholder_assets.py` generates level, HUD, enemy and minimap art
plus four sound effects, at the exact sizes the renderer needs, and writes
`assets/MANIFEST.tsv`. It is deterministic — regenerating produces
byte-identical files. Everything it makes is replaceable 1:1 by hand-drawn art
of the same dimensions.

Some assets are **generator inputs**, loaded by nothing at runtime: `Niv1.png`
and the twelve `assets/img/barre*/barre_*.png` frames. They are recorded in
`MANIFEST.tsv` as `GENERATOR INPUT` so that a sweep of "unused" assets does not
delete them and break the art pipeline.

Asset paths must resolve **case-sensitively**. `src/minimap.c` once spelled
`ressources` where the directory is `Ressources`, which failed silently at every
boot; `make verify` now catches that class of bug.

## Naming conventions

Identifiers are legacy French in the older modules: `perso` (character),
`afficher` (display), `librer`/`liberer` (free), `deplacer` (move), `vie`
(life), `barre` (health bar), `vitesse` (speed), `retour` (back), `niv` (level),
`MAJ` (update). Newer modules (`platform`, `game_state`, `collision`, `enemy`,
`hud`, `audio`) are English. Match the surrounding file rather than introducing
a parallel vocabulary mid-file. Comments are English-only.

## Direction

`docs/REFACTOR_ROADMAP.md` (git-ignored, local only) and `config/TODO.md` define
the path. v0.4.0 delivered one complete mission plus the platform layer; v0.5.0
is the SDL 2.0 backend swap, which starts by making the `platform.h` types
opaque. Explicitly out of scope: a from-scratch rewrite, a big-bang SDL2 switch
across all files, or new features layered on SDL 1.2 hacks.

Changes are tracked in `CHANGELOG.md` (Keep a Changelog + SemVer).
