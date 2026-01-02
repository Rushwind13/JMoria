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
        SESSION=$(ls -t "$LOG_DIR"/*"$1"*.jsonl 2>/dev/null | head -1)
    fi
else
    SESSION=$(ls -t "$LOG_DIR"/*.jsonl 2>/dev/null | head -1)
fi

if [ -z "$SESSION" ] || [ ! -f "$SESSION" ]; then
    echo "Error: No session file found"
    exit 1
fi

# Get the most recent dungeon state
DUNGEON=$(grep '"type":"dungeon"' "$SESSION" | tail -1)

if [ -z "$DUNGEON" ]; then
    echo "No dungeon state found in session"
    exit 1
fi

echo "=== CURRENT STATE ==="
echo ""

# Basic info
TURN=$(echo "$DUNGEON" | jq -r '.turn')
LEVEL=$(echo "$DUNGEON" | jq -r '.level')
DEPTH=$(echo "$DUNGEON" | jq -r '.depth_ft')
echo "Turn: $TURN"
echo "Level: $LEVEL ($DEPTH ft)"
echo ""

# Player
echo "=== PLAYER ==="
echo "$DUNGEON" | jq -r '.player | "Position: (\(.x), \(.y))\nHP: \(.hp) / \(.max_hp)"'
echo ""

# Decode and display the map
echo "=== MAP (21x21 viewport) ==="
echo "$DUNGEON" | jq -r '.map[] | "\(.[0]): \(.[1])"' | while read line; do
    ROW_IDX=$(echo "$line" | cut -d: -f1)
    RLE=$(echo "$line" | cut -d: -f2- | sed 's/^ //')

    # Decode RLE
    DECODED=""
    i=0
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

    printf "%2d: %s\n" "$ROW_IDX" "$DECODED"
done
echo ""

# Monsters
echo "=== MONSTERS ==="
MONSTER_COUNT=$(echo "$DUNGEON" | jq '.monsters | length')
if [ "$MONSTER_COUNT" -gt 0 ]; then
    echo "$DUNGEON" | jq -r '.monsters[] | "  \(.char) \(.name) at (\(.x),\(.y)) HP:\(.hp)"'
else
    echo "  (none visible)"
fi
echo ""

# Items on ground
echo "=== ITEMS ON GROUND ==="
ITEM_COUNT=$(echo "$DUNGEON" | jq '.items | length')
if [ "$ITEM_COUNT" -gt 0 ]; then
    echo "$DUNGEON" | jq -r '.items[] | "  \(.char) \(.name) at (\(.x),\(.y))"'
else
    echo "  (none visible)"
fi
