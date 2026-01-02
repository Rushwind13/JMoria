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
SESSION=$(ls -t "$LOG_DIR"/*.jsonl 2>/dev/null | head -1)

if [ -z "$SESSION" ]; then
    echo "No session files found in $LOG_DIR"
    exit 1
fi

echo "Monitoring: $(basename "$SESSION")"
echo "Press Ctrl+C to stop"
echo "---"

FILTER="$1"

if [ -z "$FILTER" ]; then
    # All events, pretty printed
    tail -f "$SESSION" | jq --unbuffered '.'
elif [ "$FILTER" = "combat" ]; then
    tail -f "$SESSION" | grep --line-buffered '"type":"combat"' | jq --unbuffered '.'
elif [ "$FILTER" = "move" ]; then
    tail -f "$SESSION" | grep --line-buffered '"_move"' | jq --unbuffered '{type, monster: .monster, from, to}'
elif [ "$FILTER" = "player" ]; then
    tail -f "$SESSION" | grep --line-buffered '"player_move"' | jq --unbuffered '.'
elif [ "$FILTER" = "monster" ]; then
    tail -f "$SESSION" | grep --line-buffered '"monster_move"' | jq --unbuffered '{monster, from, to, state}'
elif [ "$FILTER" = "dungeon" ]; then
    tail -f "$SESSION" | grep --line-buffered '"type":"dungeon"' | jq --unbuffered '{turn, level, player, monsters: (.monsters | length), items: (.items | length)}'
elif [ "$FILTER" = "item" ]; then
    tail -f "$SESSION" | grep --line-buffered '"type":"item"' | jq --unbuffered '.'
elif [ "$FILTER" = "brief" ]; then
    # One-line summaries
    tail -f "$SESSION" | while read line; do
        TYPE=$(echo "$line" | jq -r '.type')
        case "$TYPE" in
            turn)
                TURN=$(echo "$line" | jq -r '.turn')
                echo "--- Turn $TURN ---"
                ;;
            player_move)
                TO=$(echo "$line" | jq -r '.to | "(\(.x),\(.y))"')
                RESULT=$(echo "$line" | jq -r '.result')
                echo "Player -> $TO ($RESULT)"
                ;;
            monster_move)
                MON=$(echo "$line" | jq -r '.monster')
                TO=$(echo "$line" | jq -r '.to | "(\(.x),\(.y))"')
                echo "  $MON -> $TO"
                ;;
            combat)
                ATK=$(echo "$line" | jq -r '.attacker')
                DEF=$(echo "$line" | jq -r '.defender')
                HIT=$(echo "$line" | jq -r '.hit')
                DMG=$(echo "$line" | jq -r '.damage')
                HP=$(echo "$line" | jq -r '.defender_hp')
                if [ "$HIT" = "true" ]; then
                    echo "COMBAT: $ATK hits $DEF for $DMG dmg (HP: $HP)"
                else
                    echo "COMBAT: $ATK misses $DEF"
                fi
                ;;
            item)
                ACTION=$(echo "$line" | jq -r '.action')
                ITEM=$(echo "$line" | jq -r '.item')
                echo "ITEM: $ACTION $ITEM"
                ;;
        esac
    done
else
    # Custom grep filter
    tail -f "$SESSION" | grep --line-buffered "$FILTER" | jq --unbuffered '.'
fi
