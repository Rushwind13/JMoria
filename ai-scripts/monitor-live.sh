#!/bin/bash
# Monitor a live JMoria game session
# Usage: ./monitor-live.sh [filter]
#        ./monitor-live.sh           # all events
#        ./monitor-live.sh combat    # only combat events
#        ./monitor-live.sh move      # only movement events
#        ./monitor-live.sh dungeon   # only dungeon state

set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
LOG_DIR="$SCRIPT_DIR/../ai-logs"

# Find most recent session
SESSION=$(ls -t "$LOG_DIR"/*.log 2>/dev/null | head -1)

if [ -z "$SESSION" ]; then
    echo "No session files found in $LOG_DIR"
    exit 1
fi

echo "Monitoring: $(basename "$SESSION")"
echo "Press Ctrl+C to stop"
echo "---"

FILTER="$1"

if [ -z "$FILTER" ]; then
    tail -f "$SESSION"
elif [ "$FILTER" = "combat" ]; then
    tail -f "$SESSION" | grep --line-buffered '^COMBAT '
elif [ "$FILTER" = "move" ]; then
    tail -f "$SESSION" | grep --line-buffered '^MOVE '
elif [ "$FILTER" = "player" ]; then
    tail -f "$SESSION" | grep --line-buffered '^MOVE player'
elif [ "$FILTER" = "monster" ]; then
    tail -f "$SESSION" | grep --line-buffered '^MOVE monster'
elif [ "$FILTER" = "dungeon" ]; then
    tail -f "$SESSION" | grep --line-buffered -E '^(DUNGEON|  PLAYER|  VIEW|ENDDUNGEON)'
elif [ "$FILTER" = "item" ]; then
    tail -f "$SESSION" | grep --line-buffered '^ITEM '
elif [ "$FILTER" = "brief" ]; then
    tail -f "$SESSION" | while read line; do
        case "$line" in
            TURN*)
                TURN=$(echo "$line" | awk '{print $2}')
                echo "--- Turn $TURN ---"
                ;;
            MOVE\ player*)
                TO=$(echo "$line" | sed 's/.*to:\([^ ]*\).*/\1/')
                RESULT=$(echo "$line" | sed 's/.*result:\([^ ]*\).*/\1/')
                echo "Player -> ($TO) ($RESULT)"
                ;;
            MOVE\ monster*)
                MON=$(echo "$line" | sed 's/.*monster:\([^ ]*\).*/\1/' | tr '_' ' ')
                TO=$(echo "$line" | sed 's/.*to:\([^ ]*\).*/\1/')
                echo "  $MON -> ($TO)"
                ;;
            COMBAT*)
                ATK=$(echo "$line" | sed 's/.*attacker:\([^ ]*\).*/\1/' | tr '_' ' ')
                DEF=$(echo "$line" | sed 's/.*defender:\([^ ]*\).*/\1/' | tr '_' ' ')
                HIT=$(echo "$line" | grep -o 'hit:[^ ]*' | cut -d: -f2)
                DMG=$(echo "$line" | grep -o 'damage:[^ ]*' | cut -d: -f2)
                HP=$(echo "$line" | grep -o 'hp:[^ ]*' | cut -d: -f2)
                if [ "$HIT" = "true" ]; then
                    echo "COMBAT: $ATK hits $DEF for $DMG dmg (HP: $HP)"
                else
                    echo "COMBAT: $ATK misses $DEF"
                fi
                ;;
            ITEM*)
                ACTION=$(echo "$line" | awk '{print $2}')
                NAME=$(echo "$line" | sed 's/.*name:\([^ ]*\).*/\1/' | tr '_' ' ')
                echo "ITEM: $ACTION $NAME"
                ;;
        esac
    done
else
    # Custom grep filter
    tail -f "$SESSION" | grep --line-buffered "$FILTER"
fi
