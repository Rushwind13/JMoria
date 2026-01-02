#!/bin/bash
# List all game sessions with basic stats
# Usage: ./list-sessions.sh

set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
LOG_DIR="$SCRIPT_DIR/../ai-logs"

echo "=== GAME SESSIONS ==="
echo ""
printf "%-45s %8s %8s %s\n" "Session" "Turns" "Size" "Levels"
printf "%-45s %8s %8s %s\n" "-------" "-----" "----" "------"

# List all jsonl files (newest first)
for f in $(ls -t "$LOG_DIR"/*.jsonl 2>/dev/null); do
    NAME=$(basename "$f")
    TURNS=$(grep -c '"type":"turn"' "$f" 2>/dev/null || echo 0)
    SIZE=$(du -h "$f" | cut -f1)
    LEVELS=$(grep '"type":"dungeon"' "$f" 2>/dev/null | jq -r '.level' | sort -u | tr '\n' ',' | sed 's/,$//')

    printf "%-45s %8s %8s %s\n" "$NAME" "$TURNS" "$SIZE" "$LEVELS"
done

# Also show archived sessions
echo ""
echo "=== ARCHIVED SESSIONS ==="
ARCHIVED=$(ls "$LOG_DIR"/*.jsonl.gz 2>/dev/null | wc -l)
if [ "$ARCHIVED" -gt 0 ]; then
    for f in $(ls -t "$LOG_DIR"/*.jsonl.gz 2>/dev/null); do
        NAME=$(basename "$f")
        SIZE=$(du -h "$f" | cut -f1)
        printf "%-45s %8s %8s\n" "$NAME" "(archived)" "$SIZE"
    done
else
    echo "(none)"
fi
