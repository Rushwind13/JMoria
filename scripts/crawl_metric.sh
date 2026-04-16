#!/bin/bash
# Run the crawler bot and print summary stats.
# Automatically finds its own location and jmoria.
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJ_ROOT="$(dirname "$SCRIPT_DIR")"
OUT="$SCRIPT_DIR/bot_test_latest.txt"

# Ensure persistent crawler session exists and is ready
if ! tmux has-session -t crawler 2>/dev/null; then
    tmux new-session -d -s crawler -x 125 -y 45 "cd '$PROJ_ROOT' && exec zsh"
    sleep 1
fi

python3 -u "$SCRIPT_DIR/crawler.py" --verbose --think --persistent-session --jmoria "$PROJ_ROOT/jmoria" "$@" 2>&1 > "$OUT"
turns=$(grep -ac "\[turn" "$OUT" || echo 0)
loops=$(grep -ac "loop_detect\|loop_break" "$OUT" || echo 0)
breakouts=$(grep -ac "breakout" "$OUT" || echo 0)
doors=$(grep -ac "door_thru" "$OUT" || echo 0)
laps=$(grep -ac "lap_mapped\|lap_exit" "$OUT" || echo 0)
uwpos=$(grep -aE "wpos=" "$OUT" | sed 's/.*wpos=(\([^)]*\)).*/\1/' | sort -u | wc -l | tr -d ' ')
echo "Turns: $turns, loops: $loops, breakouts: $breakouts, doors: $doors, lap events: $laps, unique wpos: $uwpos"
tail -5 "$OUT"
# popd
