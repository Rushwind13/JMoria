#!/bin/bash
# Demo: Targeting and Ranged Combat in JMoria
# Creates items, summons a monster, targets it, and zaps it with a wand.
#
# Usage: ./scripts/bot/scenarios/demo_targeting.sh
# Requires: jmoria built (make ascii), tmux installed

set -euo pipefail

SESSION=crawler
GAME_DIR="$(cd "$(dirname "$0")/../../.." && pwd)"
DELAY=0.6

# --- helpers ---
capture() {
    sleep "$DELAY"
    echo ""
    echo "========================================"
    echo "  $1"
    echo "========================================"
    tmux capture-pane -t "$SESSION" -p
}

send_key() {
    tmux send-keys -t "$SESSION" "$1"
}

send_text() {
    tmux send-keys -t "$SESSION" -l "$1"
}

wait_for() {
    # Wait until a string appears on screen, up to N seconds
    local pattern="$1"
    local max="${2:-5}"
    local i=0
    while [ $i -lt "$max" ]; do
        if tmux capture-pane -t "$SESSION" -p 2>/dev/null | grep -q "$pattern"; then
            return 0
        fi
        sleep 0.5
        i=$((i + 1))
    done
    echo "WARNING: timed out waiting for '$pattern'" >&2
    return 1
}

create_item() {
    local name="$1"
    send_key Tab           # Tab = Ctrl+I = create item
    sleep 0.4
    wait_for "Item Name:"
    send_text "$name"
    sleep 0.2
    send_key Enter
    sleep 0.3
    # pick it up by walking right then left
    send_text "6"
    sleep 0.3
    send_text "4"
    sleep "$DELAY"
}

summon_monster() {
    local name="$1"
    send_key C-s           # Ctrl+S = summon monster
    sleep 0.4
    wait_for "Monster Name:"
    send_text "$name"
    sleep 0.2
    send_key Enter
    sleep "$DELAY"
}

# --- main ---
echo "=== JMoria Targeting & Ranged Combat Demo ==="
echo ""

# Clean up any previous session
tmux kill-session -t "$SESSION" 2>/dev/null || true
sleep 0.3

# Start the game
cd "$GAME_DIR"
if [ ! -f jmoria ]; then
    echo "ERROR: jmoria binary not found. Run 'make ascii' first." >&2
    exit 1
fi
tmux new-session -d -s "$SESSION" -x 120 -y 40 "stty -ixon; ./jmoria 2>/dev/null"
sleep 1.5

# Skip splash + character creation
send_key ' '
sleep 0.5
send_key ' '
wait_for "level 1"
capture "GAME STARTED — Dungeon Level 1"

# Enable wizard mode (Ctrl+T teleports and enables it)
send_key C-t
wait_for "Wizard Mode"
capture "WIZARD MODE ENABLED"

# Create and equip a Torch for light
create_item "Torch"
capture "PICKED UP TORCH"

send_text "w"
sleep 0.3
send_text "a"             # Torch should be slot a
wait_for "wielding"
capture "WIELDING TORCH — now we have light"

# Create a Wand of Light
create_item "Wand of Light"
capture "PICKED UP WAND OF LIGHT"

# Check inventory
echo ""
echo "--- Inventory check ---"
tmux capture-pane -t "$SESSION" -p | grep -A 5 "You are Carrying"
echo ""

# Summon a Red Jelly near the player (has MON_FLAG_HURT_BY_LIGHT)
summon_monster "Red Jelly"
capture "SUMMONED RED JELLY"

# Enter targeting mode with *
send_key '*'
sleep 0.3
wait_for "target" 3 || true   # targeting mode message or LOS line
capture "TARGETING MODE — LOS line drawn to nearest monster"

# Confirm target with .
send_text "."
sleep "$DELAY"
capture "TARGET CONFIRMED"

# Zap the wand (z, then slot letter)
send_text "z"
sleep 0.3
send_text "a"              # wand slot
sleep 3.0                  # wait for projectile to fly and hit
capture "ZAPPED WAND AT TARGET"

echo ""
echo "=== Demo Complete ==="
echo ""
echo "The tmux session '$SESSION' is still running."
echo "Attach with: tmux attach -t $SESSION"
echo "Kill with:   tmux kill-session -t $SESSION"
