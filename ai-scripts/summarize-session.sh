#!/bin/bash
# Summarize a JMoria game session from AI logs
# Usage: ./summarize-session.sh [session-file]
#        ./summarize-session.sh              # uses most recent session
#        ./summarize-session.sh session-001  # partial match

set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
LOG_DIR="$SCRIPT_DIR/../ai-logs"

# Find the session file
if [ -n "$1" ]; then
    # Try exact match first, then partial
    if [ -f "$1" ]; then
        SESSION="$1"
    elif [ -f "$LOG_DIR/$1" ]; then
        SESSION="$LOG_DIR/$1"
    else
        SESSION=$(ls -t "$LOG_DIR"/*"$1"*.jsonl 2>/dev/null | head -1)
    fi
else
    # Most recent session
    SESSION=$(ls -t "$LOG_DIR"/*.jsonl 2>/dev/null | head -1)
fi

if [ -z "$SESSION" ] || [ ! -f "$SESSION" ]; then
    echo "Error: No session file found"
    echo "Usage: $0 [session-file]"
    exit 1
fi

echo "=== SESSION SUMMARY ==="
echo "File: $(basename "$SESSION")"
echo ""

# Duration
TURNS=$(grep -c '"type":"turn"' "$SESSION" 2>/dev/null || echo 0)
echo "Duration: $TURNS turns"
echo ""

# Levels visited
echo "Levels Explored:"
grep '"type":"dungeon"' "$SESSION" | jq -r '.level' | sort -u | while read lvl; do
    depth=$((lvl * 50))
    echo "  - Level $lvl ($depth ft)"
done
echo ""

# Items collected
echo "Items Collected:"
grep '"action":"pickup"' "$SESSION" 2>/dev/null | jq -r '.item' | sort | uniq -c | sort -rn | while read count item; do
    echo "  $count x $item"
done
if ! grep -q '"action":"pickup"' "$SESSION" 2>/dev/null; then
    echo "  (none)"
fi
echo ""

# Combat summary
echo "Combat Encounters:"
if grep -q '"type":"combat"' "$SESSION" 2>/dev/null; then
    grep '"type":"combat"' "$SESSION" | jq -r '.defender' | sort | uniq -c | sort -rn | while read count target; do
        echo "  $count attacks on $target"
    done
else
    echo "  (none)"
fi
echo ""

# Monsters killed (workaround for Bug #156: check HP instead of killed flag)
echo "Monsters Defeated:"
KILLS=$(grep '"type":"combat"' "$SESSION" 2>/dev/null | jq -r 'select(.defender != "Player") | select(.defender_hp == 0 or .defender_hp < 0.01) | .defender' | sort | uniq -c | sort -rn)
if [ -n "$KILLS" ]; then
    echo "$KILLS" | while read count monster; do
        echo "  $count x $monster"
    done
else
    echo "  (none)"
fi
echo ""

# Final state
echo "Final State:"
FINAL=$(grep '"type":"dungeon"' "$SESSION" | tail -1)
if [ -n "$FINAL" ]; then
    LEVEL=$(echo "$FINAL" | jq -r '.level')
    DEPTH=$(echo "$FINAL" | jq -r '.depth_ft')
    HP=$(echo "$FINAL" | jq -r '.player.hp')
    MAX_HP=$(echo "$FINAL" | jq -r '.player.max_hp')
    PX=$(echo "$FINAL" | jq -r '.player.x')
    PY=$(echo "$FINAL" | jq -r '.player.y')
    echo "  Level: $LEVEL ($DEPTH ft)"
    echo "  HP: $HP / $MAX_HP"
    echo "  Position: ($PX, $PY)"
else
    echo "  (no dungeon data)"
fi
echo ""

# Check if player died
if grep -q '"type":"session_end"' "$SESSION"; then
    echo "Outcome: Session ended normally"
else
    # Check last combat - did player get hit and HP go to 0?
    LAST_PLAYER_HP=$(grep '"type":"dungeon"' "$SESSION" | tail -1 | jq -r '.player.hp')
    if [ "$LAST_PLAYER_HP" = "0" ] || [ "$(echo "$LAST_PLAYER_HP < 1" | bc -l 2>/dev/null)" = "1" ]; then
        echo "Outcome: DIED"
    else
        echo "Outcome: Survived (session may still be active)"
    fi
fi
