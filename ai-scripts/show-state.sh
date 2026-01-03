#!/bin/bash
# Show current game state from most recent log entry
# Usage: ./show-state.sh [session-file]

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
    exit 1
fi

# Decode RLE function
decode_rle() {
    local RLE="$1"
    local DECODED=""
    local i=0
    while [ $i -lt ${#RLE} ]; do
        CHAR="${RLE:$i:1}"
        i=$((i + 1))

        # Collect digits
        COUNT=""
        while [ $i -lt ${#RLE} ] && [[ "${RLE:$i:1}" =~ [0-9] ]]; do
            COUNT="${COUNT}${RLE:$i:1}"
            i=$((i + 1))
        done

        # Expand
        if [ -z "$COUNT" ]; then
            COUNT=1
        fi
        for ((j=0; j<COUNT; j++)); do
            DECODED="${DECODED}${CHAR}"
        done
    done
    echo "$DECODED"
}

echo "=== CURRENT STATE ==="
echo ""

# Find the last DUNGEON block
LAST_DUNGEON_LINE=$(grep -n '^DUNGEON ' "$SESSION" | tail -1 | cut -d: -f1)

if [ -z "$LAST_DUNGEON_LINE" ]; then
    echo "No dungeon state found in session"
    exit 1
fi

# Extract the dungeon block
DUNGEON_BLOCK=$(sed -n "${LAST_DUNGEON_LINE},/^ENDDUNGEON/p" "$SESSION")

# Basic info
DUNGEON_HEADER=$(echo "$DUNGEON_BLOCK" | head -1)
TURN=$(echo "$DUNGEON_HEADER" | sed 's/.*turn:\([0-9]*\).*/\1/')
LEVEL=$(echo "$DUNGEON_HEADER" | sed 's/.*level:\([0-9]*\).*/\1/')
DEPTH=$((LEVEL * 50))
echo "Turn: $TURN"
echo "Level: $LEVEL ($DEPTH ft)"
echo ""

# Player
echo "=== PLAYER ==="
PLAYER_LINE=$(echo "$DUNGEON_BLOCK" | grep '  PLAYER')
if [ -n "$PLAYER_LINE" ]; then
    POS=$(echo "$PLAYER_LINE" | awk '{print $2}')
    HP_INFO=$(echo "$PLAYER_LINE" | grep -o 'hp:[0-9]*/[0-9]*' | sed 's/hp://')
    echo "Position: ($POS)"
    echo "HP: $HP_INFO"
fi
echo ""

# Map
echo "=== MAP (21x21 viewport) ==="
echo "$DUNGEON_BLOCK" | grep '    [0-9]*:' | while read line; do
    ROW_IDX=$(echo "$line" | sed 's/^[[:space:]]*//' | cut -d: -f1)
    RLE=$(echo "$line" | cut -d: -f2- | sed 's/^ //')
    DECODED=$(decode_rle "$RLE")
    printf "%2d: %s\n" "$ROW_IDX" "$DECODED"
done
echo ""

# Monsters
echo "=== MONSTERS ==="
MONSTERS=$(echo "$DUNGEON_BLOCK" | grep '  MON ')
if [ -n "$MONSTERS" ]; then
    echo "$MONSTERS" | while read line; do
        NAME=$(echo "$line" | awk '{print $2}' | tr '_' ' ')
        CHAR=$(echo "$line" | awk '{print $3}')
        POS=$(echo "$line" | awk '{print $4}')
        HP=$(echo "$line" | grep -o 'hp:[0-9]*' | cut -d: -f2)
        echo "  $CHAR $NAME at ($POS) HP:$HP"
    done
else
    echo "  (none visible)"
fi
echo ""

# Items
echo "=== ITEMS ON GROUND ==="
ITEMS=$(echo "$DUNGEON_BLOCK" | grep '  ITEM ')
if [ -n "$ITEMS" ]; then
    echo "$ITEMS" | while read line; do
        NAME=$(echo "$line" | awk '{print $2}' | tr '_' ' ')
        CHAR=$(echo "$line" | awk '{print $3}')
        POS=$(echo "$line" | awk '{print $4}')
        echo "  $CHAR $NAME at ($POS)"
    done
else
    echo "  (none visible)"
fi
