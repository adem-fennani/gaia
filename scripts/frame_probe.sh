#!/usr/bin/env bash
# Capture rendered frames from level 1 headlessly, for before/after comparison.
#
# There is no test suite, so refactors that must not change behaviour (the
# platform wrappers, the state machine split) need some way to prove it. This
# temporarily patches main_menu.c to boot straight into the level, walk both
# players across it, dump frames at fixed positions, and exit -- then restores
# the file. Nothing is left behind in the source tree.
#
#   scripts/frame_probe.sh <output-dir>
#
# Compare two runs with `cmp` or sha256sum. Identical frames mean the render
# path is unchanged.
set -euo pipefail

OUT=${1:?usage: frame_probe.sh <output-dir>}
cd "$(dirname "$0")/.."
mkdir -p "$OUT"

SRC=src/game.c
BAK=$(mktemp)
cp "$SRC" "$BAK"
# Always put the source back, even if the build or run fails.
trap 'cp "$BAK" "$SRC"; rm -f "$BAK"; make -s pro >/dev/null 2>&1 || true' EXIT

python3 - "$SRC" <<'PY'
import sys
path = sys.argv[1]
s = open(path).read()
# Boot into level 1 instead of the menu.
# Boot into level 1 instead of the menu, and run its on_enter so the level is
# initialised the way a real transition would leave it.
s = s.replace("  g_game.state = ST_MENU;\n  g_game.next = ST_MENU;",
              "  g_game.state = ST_LEVEL1;\n  g_game.next = ST_LEVEL1;\n"
              "  level1_state.on_enter(&g_game);", 1)
# Drive the camera and dump frames. Placed after the flip so what is captured
# is exactly what was presented. The flip is spelled SDL_Flip before the
# platform wrappers land and plat_flip after, so accept either.
flip = next((f for f in ("    plat_flip(g_game.screen);",
                         "    plat_flip(screen);", "    SDL_Flip(screen);")
             if f in s), None)
if flip is None:
    sys.exit("frame_probe: could not find the screen flip to hook")
s = s.replace(flip, flip + """
    {
      static int probe_n = 0;
      const char *probe_dir = getenv("GAIA_PROBE");
      if (probe_dir != NULL) {
        char probe_path[1024];
        /* wx is authoritative since the collision rewrite; pos_background is
         * derived from it for drawing. */
        g_game.p.wx = 60 + probe_n * 24;
        g_game.p1.wx = 200 + probe_n * 24;
        perso_sync_rect(&g_game.p);
        perso_sync_rect(&g_game.p1);
        if (probe_n == 0 || probe_n == 20 || probe_n == 40 ||
            probe_n == 60 || probe_n == 80) {
          snprintf(probe_path, sizeof(probe_path), "%s/frame_%02d.bmp",
                   probe_dir, probe_n);
          SDL_SaveBMP(g_game.screen, probe_path);
        }
        if (++probe_n > 84) {
          g_game.running = 0;
        }
      }
    }""", 1)
open(path, "w").write(s)
PY

make -s pro >/dev/null
rm -f "$OUT"/frame_*.bmp
GAIA_PROBE="$OUT" timeout 30 env SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy \
    ./pro >/dev/null 2>&1 || true
count=$(ls -1 "$OUT"/frame_*.bmp 2>/dev/null | wc -l)
if [ "$count" -eq 0 ]; then
    echo "frame_probe: no frames captured" >&2
    exit 1
fi
echo "captured $count frames into $OUT"
