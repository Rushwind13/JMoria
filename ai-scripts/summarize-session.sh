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
    if [ -f "$1" ]; then
        SESSION="$1"
    elif [ -f "$LOG_DIR/$1" ]; then
        SESSION="$LOG_DIR/$1"
    else
        SESSION=$(ls -t "$LOG_DIR"/*"$1"*.log 2>/dev/null | head -1)
    fi
else
    SESSION=$(ls -t "$LOG_DIR"/*.log 2>/dev/null | head -1)
fi

if [ -z "$SESSION" ] || [ ! -f "$SESSION" ]; then
    echo "Error: No session file found"
    echo "Usage: $0 [session-file]"
    exit 1
fi

echo "=== SESSION SUMMARY ==="
echo "File: $(basename "$SESSION")"
echo ""

TURNS=$(grep -c '^TURN ' "$SESSION" 2>/dev/null || echo 0)
echo "Duration: $TURNS turns"
echo ""

echo "Levels Explored:"
grep '^DUNGEON ' "$SESSION" | sed 's/.*level:\([0-9]*\).*/\1/' | sort -u | while read lvl; do
    depth=$((lvl * 50))
    echo "  - Level $lvl ($depth ft)"
done
echo ""

echo "Items Collected:"
if grep -q '^ITEM pickup' "$SESSION" 2>/dev/null; then
    grep '^ITEM pickup' "$SESSION" | sed 's/.*name:\([^ ]*\).*/\1/' | tr '_' ' ' | sort | uniq -c | sort -rn | while read count item; do
        echo "  $count x $item"
    done
else
    echo "  (none)"
fi
echo ""

echo "Combat Encounters:"
if grep -q '^COMBAT ' "$SESSION" 2>/dev/null; then
    grep '^COMBAT ' "$SESSION" | sed 's/.*defender:\([^ ]*\).*/\1/' | tr '_' ' ' | sort | uniq -c | sort -rn | while read count target; do
        echo "  $count attacks on $target"
    done
else
    echo "  (none)"
fi
echo ""

echo "Monsters Defeated:"
KILLS=$(grep '^COMBAT ' "$SESSION" 2>/dev/null | grep 'killed:true' | sed 's/.*defender:\([^ ]*\).*/\1/' | tr '_' ' ' | sort | uniq -c | sort -rn)
if [ -n "$KILLS" ]; then
    echo "$KILLS" | while read count monster; do
        echo "  $count x $monster"
    done
else
    echo "  (none)"
fi
echo ""

echo "Final State:"
LAST_DUNGEON_LINE=$(grep -n '^DUNGEON ' "$SESSION" | tail -1 | cut -d: -f1)
if [ -n "$LAST_DUNGEON_LINE" ]; then
    LEVEL=$(sed -n "${LAST_DUNGEON_LINE}p" "$SESSION" | sed 's/.*level:\([0-9]*\).*/\1/')
    DEPTH=$((LEVEL * 50))
    PLAYER_LINE=$(sed -n "${LAST_DUNGEON_LINE},\$p" "$SESSION" | grep '  PLAYER' | head -1)
    if [ -n "$PLAYER_LINE" ]; then
        POS=$(echo "$PLAYER_LINE" | awk '{print $2}')
        HP_INFO=$(echo "$PLAYER_LINE" | grep -o 'hp:[0-9]*/[0-9]*')
        echo "  Level: $LEVEL ($DEPTH ft)"
        echo "  HP: $(echo "$HP_INFO" | sed 's/hp://')"
        echo "  Position: ($POS)"
    else
        echo "  Level: $LEVEL ($DEPTH ft)"
    fi
else
    echo "  (no dungeon data)"
fi
echo ""

# Check outcome
if grep -q '^DEATH ' "$SESSION" 2>/dev/null; then
    KILLER=$(grep '^DEATH ' "$SESSION" | tail -1 | sed 's/.*killed_by:\([^ ]*\).*/\1/' | tr '_' ' ')
    echo "Outcome: DIED (killed by $KILLER)"
elif grep -q '^SESSION_END' "$SESSION"; then
    echo "Outcome: Session ended normally"
else
    echo "Outcome: Survived (session may still be active)"
fi
