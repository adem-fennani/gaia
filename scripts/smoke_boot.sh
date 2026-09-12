#!/usr/bin/env bash
# Headless boot check. There is no test suite, so this is the runtime gate.
#
# The game has no clean exit path from its own loop without input, so success
# is `timeout` killing it: exit 124 means it reached the main loop and stayed
# there. Anything else means it died during init.
#
# It also fails on asset-load failures, which are otherwise silent: on a
# missing image load_image_safe() substitutes a magenta checkerboard and
# returns non-NULL, so the game boots "fine" and renders placeholder art. Only
# the log line distinguishes that from a real load.
set -uo pipefail

cd "$(dirname "$0")/.."

if [ ! -x ./pro ]; then
    echo "smoke: ./pro not built; run 'make pro' first" >&2
    exit 1
fi

LOG=$(mktemp)
trap 'rm -f "$LOG"' EXIT

timeout 5 env SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy ./pro >"$LOG" 2>&1
rc=$?

if [ "$rc" -ne 124 ]; then
    echo "smoke: expected exit 124 (timeout killed the game loop), got $rc" >&2
    echo "--- output ---" >&2
    cat "$LOG" >&2
    exit 1
fi

# Assets are loaded up front in load_game_resources(), so a clean boot log
# means every asset for every state resolved.
# Covers the platform layer's messages ("[plat_image_load] Failed to load
# ...", "[plat_init] Mix_OpenAudio failed ...") as well as the older wording
# still emitted from perso.c and minimap.c.
BAD='Failed to load|Unable to load|Error loading|FAIL AUDIO|failed:|Mix_OpenAudio failed'
if grep -qE "$BAD" "$LOG"; then
    echo "smoke: reached the game loop, but initialisation reported errors:" >&2
    grep -nE "$BAD" "$LOG" >&2
    exit 1
fi

echo "OK: booted headless, reached the game loop, no asset-load failures"
