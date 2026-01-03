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

# List .log files (newest first)
for f in $(ls -t "$LOG_DIR"/*.log 2>/dev/null); do
    NAME=$(basename "$f")
    TURNS=$(grep -c '^TURN ' "$f" 2>/dev/null || echo 0)
    SIZE=$(du -h "$f" | cut -f1)
    LEVELS=$(grep '^DUNGEON ' "$f" 2>/dev/null | sed 's/.*level:\([0-9]*\).*/\1/' | sort -u | tr '\n' ',' | sed 's/,$//')

    printf "%-45s %8s %8s %s\n" "$NAME" "$TURNS" "$SIZE" "$LEVELS"
done

# Show archived sessions
echo ""
echo "=== ARCHIVED SESSIONS ==="
ARCHIVED=$(ls "$LOG_DIR"/*.log.gz 2>/dev/null | wc -l)

if [ "$ARCHIVED" -gt 0 ]; then
    for f in $(ls -t "$LOG_DIR"/*.log.gz 2>/dev/null); do
        NAME=$(basename "$f")
        SIZE=$(du -h "$f" | cut -f1)
        printf "%-45s %8s %8s\n" "$NAME" "(archived)" "$SIZE"
    done
else
    echo "(none)"
fi
