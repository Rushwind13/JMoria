#!/usr/bin/env python3
"""
Deep Dive Session Analysis for JMoria AI Logs

Usage: ./deep-dive.py [session-file]
       ./deep-dive.py                    # uses most recent session
       ./deep-dive.py session-008        # partial match
       ./deep-dive.py --map              # show map snapshots
       ./deep-dive.py --timeline         # show event timeline
"""

import json
import os
import sys
import glob
import argparse
from collections import Counter, defaultdict
from pathlib import Path


def find_session(pattern=None):
    """Find session file by pattern or return most recent."""
    script_dir = Path(__file__).parent
    log_dir = script_dir / ".." / "ai-logs"

    if pattern:
        # Try exact path first
        if os.path.isfile(pattern):
            return pattern
        # Try in log dir
        full_path = log_dir / pattern
        if full_path.is_file():
            return str(full_path)
        # Try partial match
        matches = sorted(glob.glob(str(log_dir / f"*{pattern}*.jsonl")))
        if matches:
            return matches[-1]
        return None

    # Most recent
    sessions = sorted(glob.glob(str(log_dir / "*.jsonl")))
    return sessions[-1] if sessions else None


def load_events(filepath):
    """Load all events from a session file."""
    events = []
    with open(filepath, 'r') as f:
        for line in f:
            line = line.strip()
            if line:
                try:
                    events.append(json.loads(line))
                except json.JSONDecodeError:
                    pass
    return events


def decode_rle(s):
    """Decode RLE-encoded map row."""
    result = []
    i = 0
    while i < len(s):
        c = s[i]
        i += 1
        count = 0
        while i < len(s) and s[i].isdigit():
            count = count * 10 + int(s[i])
            i += 1
        if count == 0:
            count = 1
        result.append(c * count)
    return ''.join(result)


def analyze_session(events, show_map=False, show_timeline=False):
    """Perform deep analysis on session events."""

    # Categorize events
    dungeons = [e for e in events if e.get('type') == 'dungeon']
    turns = [e for e in events if e.get('type') == 'turn']
    player_moves = [e for e in events if e.get('type') == 'player_move']
    monster_moves = [e for e in events if e.get('type') == 'monster_move']
    combats = [e for e in events if e.get('type') == 'combat']
    items = [e for e in events if e.get('type') == 'item']

    print("=" * 60)
    print("DEEP DIVE SESSION ANALYSIS")
    print("=" * 60)

    # Overview
    print(f"\n{'='*20} OVERVIEW {'='*20}")
    print(f"  Total events: {len(events)}")
    print(f"  Turns played: {len(turns)}")
    print(f"  Player moves: {len(player_moves)}")
    print(f"  Monster moves: {len(monster_moves)}")
    print(f"  Combat events: {len(combats)}")
    print(f"  Item events: {len(items)}")

    # Level progression
    if dungeons:
        levels = {}
        for d in dungeons:
            lvl = d.get('level', 0)
            turn = d.get('turn', 0)
            if lvl not in levels:
                levels[lvl] = {'first_turn': turn, 'last_turn': turn, 'count': 0}
            levels[lvl]['last_turn'] = turn
            levels[lvl]['count'] += 1

        print(f"\n{'='*20} LEVELS {'='*20}")
        for lvl in sorted(levels.keys()):
            info = levels[lvl]
            depth = lvl * 50
            print(f"  Level {lvl} ({depth} ft): turns {info['first_turn']}-{info['last_turn']} ({info['count']} snapshots)")

    # Dungeon info from first dungeon on each level
    if dungeons:
        seen_levels = set()
        print(f"\n{'='*20} DUNGEON LAYOUT {'='*20}")
        for d in dungeons:
            lvl = d.get('level', 0)
            if lvl not in seen_levels:
                seen_levels.add(lvl)
                print(f"  Level {lvl}: {d.get('rooms', '?')} rooms, {d.get('hallways', '?')} hallways")

    # Map display
    if show_map and dungeons:
        print(f"\n{'='*20} MAP SNAPSHOTS {'='*20}")
        # Show first map of each level
        seen_levels = set()
        for d in dungeons:
            lvl = d.get('level', 0)
            if lvl not in seen_levels:
                seen_levels.add(lvl)
                print(f"\n  Level {lvl} (turn {d.get('turn')}):")
                print("  Legend: @ player, # wall, . floor, + door, > stairs")
                for row in d.get('map', []):
                    if isinstance(row, list) and len(row) >= 2:
                        decoded = decode_rle(row[1])
                        print(f"    {decoded}")

    # Items
    print(f"\n{'='*20} ITEMS {'='*20}")
    if items:
        for item in items:
            action = item.get('action', '?')
            name = item.get('item', '?')
            pos = item.get('position', {})
            print(f"  [{action.upper():8}] {name} at ({pos.get('x')},{pos.get('y')})")
    else:
        print("  No item events logged")

    # Show items visible at start
    if dungeons:
        first_items = dungeons[0].get('items', [])
        if first_items:
            print(f"\n  Items visible at session start ({len(first_items)}):")
            for i in first_items[:10]:
                print(f"    - {i.get('name')} '{i.get('char')}' at ({i.get('x')},{i.get('y')})")
            if len(first_items) > 10:
                print(f"    ... and {len(first_items) - 10} more")

    # Monsters
    print(f"\n{'='*20} MONSTERS {'='*20}")
    if dungeons:
        # Unique monsters seen across session
        all_monsters = set()
        for d in dungeons:
            for m in d.get('monsters', []):
                all_monsters.add(m.get('name'))
        print(f"  Unique monster types seen: {len(all_monsters)}")
        for name in sorted(all_monsters):
            print(f"    - {name}")

    # Monster activity
    if monster_moves:
        by_monster = defaultdict(lambda: {'moves': 0, 'states': Counter()})
        for m in monster_moves:
            name = m.get('monster', 'Unknown')
            by_monster[name]['moves'] += 1
            by_monster[name]['states'][m.get('state', 'unknown')] += 1

        print(f"\n  Monster activity:")
        for name, data in sorted(by_monster.items(), key=lambda x: -x[1]['moves']):
            states = ', '.join(f"{s}:{c}" for s, c in data['states'].most_common())
            print(f"    - {name}: {data['moves']} moves ({states})")

    # Combat
    print(f"\n{'='*20} COMBAT {'='*20}")
    if combats:
        player_attacks = [c for c in combats if c.get('attacker') == 'Player']
        monster_attacks = [c for c in combats if c.get('defender') == 'Player']

        print(f"  Player attacks: {len(player_attacks)}")
        player_hits = sum(1 for c in player_attacks if c.get('hit'))
        player_dmg = sum(c.get('damage', 0) for c in player_attacks if c.get('hit'))
        print(f"    Hits: {player_hits}, Misses: {len(player_attacks) - player_hits}")
        print(f"    Total damage dealt: {player_dmg}")

        print(f"\n  Monster attacks on player: {len(monster_attacks)}")
        monster_hits = sum(1 for c in monster_attacks if c.get('hit'))
        monster_dmg = sum(c.get('damage', 0) for c in monster_attacks if c.get('hit'))
        print(f"    Hits: {monster_hits}, Misses: {len(monster_attacks) - monster_hits}")
        print(f"    Total damage taken: {monster_dmg}")

        # Kills
        kills = [c for c in combats if c.get('defender') != 'Player' and c.get('defender_hp', 1) <= 0]
        if kills:
            print(f"\n  Monsters killed:")
            kill_counts = Counter(c.get('defender') for c in kills)
            for monster, count in kill_counts.most_common():
                print(f"    - {monster} x{count}")

        # Combat log
        print(f"\n  Combat log:")
        for c in combats:
            hit = "HIT" if c.get('hit') else "MISS"
            attacker = c.get('attacker', '?')
            defender = c.get('defender', '?')
            dmg = c.get('damage', 0)
            hp = c.get('defender_hp', '?')
            killed = " [KILLED]" if c.get('defender_hp', 1) <= 0 else ""
            print(f"    {attacker} -> {defender}: {hit} (dmg:{dmg}) HP->{hp}{killed}")
    else:
        print("  No combat this session")

    # Player status
    print(f"\n{'='*20} PLAYER STATUS {'='*20}")
    if dungeons:
        p_first = dungeons[0].get('player', {})
        p_last = dungeons[-1].get('player', {})
        print(f"  Start: HP {p_first.get('hp')}/{p_first.get('max_hp')} at ({p_first.get('x')},{p_first.get('y')})")
        print(f"  End:   HP {p_last.get('hp')}/{p_last.get('max_hp')} at ({p_last.get('x')},{p_last.get('y')})")

        # Track HP changes
        hp_changes = []
        prev_hp = None
        for d in dungeons:
            hp = d.get('player', {}).get('hp')
            turn = d.get('turn', 0)
            if prev_hp is not None and hp != prev_hp:
                diff = hp - prev_hp
                hp_changes.append((turn, diff, hp))
            prev_hp = hp

        if hp_changes:
            print(f"\n  HP changes:")
            for turn, diff, hp in hp_changes[-10:]:  # Last 10 changes
                sign = "+" if diff > 0 else ""
                print(f"    Turn {turn}: {sign}{diff} -> HP {hp}")

    # Movement results
    if player_moves:
        results = Counter(m.get('result') for m in player_moves)
        print(f"\n  Movement outcomes:")
        for r, c in results.most_common():
            print(f"    - {r}: {c}")

    # Outcome
    print(f"\n{'='*20} OUTCOME {'='*20}")

    # Check for death (HP 0 in last combat or dungeon)
    died = False
    killer = None

    # Check combat for killing blow
    if combats:
        last_player_combat = [c for c in combats if c.get('defender') == 'Player']
        if last_player_combat:
            last = last_player_combat[-1]
            if last.get('defender_hp', 1) <= 0:
                died = True
                killer = last.get('attacker', 'Unknown')

    # Double-check with dungeon HP
    if dungeons:
        final_hp = dungeons[-1].get('player', {}).get('hp', 1)
        if final_hp <= 0:
            died = True

    if died:
        print(f"  DIED!")
        if killer:
            print(f"  Killed by: {killer}")
        # Note missing event
        deaths = [e for e in events if e.get('type') == 'player_death']
        if not deaths:
            print(f"  (Note: player_death event not logged - this should be added)")
    else:
        print(f"  Survived!")

    # Check for level changes without events
    if dungeons:
        prev_level = None
        level_changes = []
        for d in dungeons:
            lvl = d.get('level')
            turn = d.get('turn', 0)
            if prev_level is not None and lvl != prev_level:
                level_changes.append((turn, prev_level, lvl))
            prev_level = lvl

        if level_changes:
            print(f"\n  Level changes detected:")
            for turn, from_lvl, to_lvl in level_changes:
                direction = "descended" if to_lvl > from_lvl else "ascended"
                print(f"    Turn {turn}: {direction} from L{from_lvl} to L{to_lvl}")

            # Check if level_change events exist
            level_events = [e for e in events if e.get('type') == 'level_change']
            if not level_events:
                print(f"  (Note: level_change events not logged - this should be added)")

    # Timeline
    if show_timeline:
        print(f"\n{'='*20} EVENT TIMELINE {'='*20}")
        for e in events[:50]:  # First 50 events
            etype = e.get('type', 'unknown')
            if etype == 'turn':
                print(f"  --- Turn {e.get('turn')} ---")
            elif etype == 'player_move':
                fr = e.get('from', {})
                to = e.get('to', {})
                print(f"  Player: ({fr.get('x')},{fr.get('y')}) -> ({to.get('x')},{to.get('y')}) [{e.get('result')}]")
            elif etype == 'combat':
                hit = "HIT" if e.get('hit') else "MISS"
                print(f"  Combat: {e.get('attacker')} -> {e.get('defender')}: {hit}")
            elif etype == 'item':
                print(f"  Item: {e.get('action')} {e.get('item')}")
            elif etype == 'monster_move':
                print(f"  Monster: {e.get('monster')} moved")
        if len(events) > 50:
            print(f"  ... and {len(events) - 50} more events")


def main():
    parser = argparse.ArgumentParser(description='Deep dive analysis of JMoria session logs')
    parser.add_argument('session', nargs='?', help='Session file or partial name')
    parser.add_argument('--map', action='store_true', help='Show map snapshots')
    parser.add_argument('--timeline', action='store_true', help='Show event timeline')
    args = parser.parse_args()

    session_file = find_session(args.session)
    if not session_file:
        print("Error: No session file found")
        print("Usage: deep-dive.py [session-file]")
        sys.exit(1)

    print(f"Analyzing: {os.path.basename(session_file)}")

    events = load_events(session_file)
    if not events:
        print("Error: No events found in session file")
        sys.exit(1)

    analyze_session(events, show_map=args.map, show_timeline=args.timeline)


if __name__ == '__main__':
    main()
