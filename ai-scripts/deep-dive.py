#!/usr/bin/env python3
"""
Deep Dive Session Analysis for JMoria AI Logs

Usage: ./deep-dive.py [session-file]
       ./deep-dive.py                    # uses most recent session
       ./deep-dive.py session-008        # partial match
       ./deep-dive.py --map              # show map snapshots
       ./deep-dive.py --timeline         # show event timeline
"""

import os
import sys
import glob
import argparse
import re
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
        matches = sorted(glob.glob(str(log_dir / f"*{pattern}*.log")))
        if matches:
            return matches[-1]
        return None

    # Most recent
    sessions = sorted(glob.glob(str(log_dir / "*.log")))
    return sessions[-1] if sessions else None


def parse_key_values(text):
    """Parse 'key:value key2:value2' format into a dict."""
    result = {}
    for match in re.finditer(r'(\w+):([^\s]+)', text):
        key, value = match.groups()
        try:
            result[key] = int(value.rstrip('ft'))
        except ValueError:
            if ',' in value:
                parts = value.split(',')
                if len(parts) == 2:
                    try:
                        result[key] = {'x': int(parts[0]), 'y': int(parts[1])}
                    except ValueError:
                        result[key] = value
                else:
                    result[key] = value
            else:
                result[key] = value
    return result


def load_events(filepath):
    """Load all events from a session file."""
    events = []

    with open(filepath, 'r', encoding='latin-1') as f:
        lines = f.readlines()

    i = 0
    while i < len(lines):
        line = lines[i].rstrip()

        if not line or line.startswith('  '):
            i += 1
            continue

        parts = line.split(None, 1)
        if not parts:
            i += 1
            continue

        event_type = parts[0]
        rest = parts[1] if len(parts) > 1 else ''

        if event_type == 'SESSION_START':
            events.append({'type': 'session_start', 'timestamp': rest})
        elif event_type == 'SESSION_END':
            events.append({'type': 'session_end', 'timestamp': rest})
        elif event_type == 'TURN':
            events.append({'type': 'turn', 'turn': int(rest)})
        elif event_type == 'LEVEL_CHANGE':
            kv = parse_key_values(rest)
            events.append({
                'type': 'level_change',
                'direction': rest.split()[0] if rest else 'unknown',
                'from_level': kv.get('from', 0),
                'to_level': kv.get('to', 0),
                'depth_ft': kv.get('depth', 0)
            })
        elif event_type == 'MOVE':
            kv = parse_key_values(rest)
            if rest.startswith('player'):
                events.append({
                    'type': 'player_move',
                    'from': kv.get('from', {}),
                    'to': kv.get('to', {}),
                    'result': kv.get('result', 'unknown')
                })
            elif rest.startswith('monster:'):
                events.append({
                    'type': 'monster_move',
                    'monster': kv.get('monster', '').replace('_', ' '),
                    'from': kv.get('from', {}),
                    'to': kv.get('to', {}),
                    'state': kv.get('state', 'unknown')
                })
        elif event_type == 'COMBAT':
            kv = parse_key_values(rest)
            events.append({
                'type': 'combat',
                'attacker': kv.get('attacker', '').replace('_', ' '),
                'defender': kv.get('defender', '').replace('_', ' '),
                'hit': kv.get('hit') == 'true',
                'damage': kv.get('damage', 0),
                'defender_hp': kv.get('hp', 0),
                'killed': kv.get('killed') == 'true'
            })
        elif event_type == 'ITEM':
            kv = parse_key_values(rest)
            pos = kv.get('pos', {})
            events.append({
                'type': 'item',
                'action': rest.split()[0] if rest else 'unknown',
                'item': kv.get('name', '').replace('_', ' '),
                'count': kv.get('count', 1),
                'position': pos if isinstance(pos, dict) else {}
            })
        elif event_type == 'DEATH':
            kv = parse_key_values(rest)
            pos = kv.get('pos', {})
            events.append({
                'type': 'player_death',
                'killed_by': kv.get('killed_by', '').replace('_', ' '),
                'level': kv.get('level', 0),
                'position': pos if isinstance(pos, dict) else {}
            })
        elif event_type == 'DUNGEON':
            kv = parse_key_values(rest)
            dungeon = {
                'type': 'dungeon',
                'turn': kv.get('turn', 0),
                'level': kv.get('level', 0),
                'depth_ft': kv.get('depth', 0),
                'map': [],
                'monsters': [],
                'items': []
            }
            i += 1

            while i < len(lines) and not lines[i].startswith('ENDDUNGEON'):
                subline = lines[i].rstrip()
                if subline.startswith('  VIEW'):
                    parts = subline.split()
                    if len(parts) >= 3:
                        coords = parts[1].split(',')
                        size = parts[2].split('x')
                        dungeon['view'] = {
                            'x': int(coords[0]) if coords else 0,
                            'y': int(coords[1]) if len(coords) > 1 else 0,
                            'w': int(size[0]) if size else 0,
                            'h': int(size[1]) if len(size) > 1 else 0
                        }
                elif subline.startswith('  PLAYER'):
                    parts = subline.split()
                    if len(parts) >= 3:
                        coords = parts[1].split(',')
                        hp_part = parts[2] if len(parts) > 2 else 'hp:0/0'
                        hp_match = re.search(r'hp:(\d+)/(\d+)', hp_part)
                        dungeon['player'] = {
                            'x': int(coords[0]) if coords else 0,
                            'y': int(coords[1]) if len(coords) > 1 else 0,
                            'hp': int(hp_match.group(1)) if hp_match else 0,
                            'max_hp': int(hp_match.group(2)) if hp_match else 0
                        }
                elif subline.startswith('  ROOMS'):
                    dungeon['rooms'] = int(subline.split()[1]) if len(subline.split()) > 1 else 0
                elif subline.startswith('  HALLS'):
                    dungeon['hallways'] = int(subline.split()[1]) if len(subline.split()) > 1 else 0
                elif subline.startswith('    ') and ':' in subline:
                    match = re.match(r'\s+(\d+):\s*(.+)', subline)
                    if match:
                        dungeon['map'].append([int(match.group(1)), match.group(2)])
                elif subline.startswith('  MON'):
                    parts = subline.split()
                    if len(parts) >= 5:
                        coords = parts[3].split(',')
                        hp_match = re.search(r'hp:(\d+)', subline)
                        dungeon['monsters'].append({
                            'name': parts[1].replace('_', ' '),
                            'char': parts[2],
                            'x': int(coords[0]) if coords else 0,
                            'y': int(coords[1]) if len(coords) > 1 else 0,
                            'hp': int(hp_match.group(1)) if hp_match else 0
                        })
                elif subline.startswith('  ITEM'):
                    parts = subline.split()
                    if len(parts) >= 4:
                        coords = parts[3].split(',')
                        dungeon['items'].append({
                            'name': parts[1].replace('_', ' '),
                            'char': parts[2],
                            'x': int(coords[0]) if coords else 0,
                            'y': int(coords[1]) if len(coords) > 1 else 0
                        })
                i += 1

            events.append(dungeon)

        i += 1

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

    dungeons = [e for e in events if e.get('type') == 'dungeon']
    turns = [e for e in events if e.get('type') == 'turn']
    player_moves = [e for e in events if e.get('type') == 'player_move']
    monster_moves = [e for e in events if e.get('type') == 'monster_move']
    combats = [e for e in events if e.get('type') == 'combat']
    items = [e for e in events if e.get('type') == 'item']
    deaths = [e for e in events if e.get('type') == 'player_death']

    print("=" * 60)
    print("DEEP DIVE SESSION ANALYSIS")
    print("=" * 60)

    print(f"\n{'='*20} OVERVIEW {'='*20}")
    print(f"  Total events: {len(events)}")
    print(f"  Turns played: {len(turns)}")
    print(f"  Player moves: {len(player_moves)}")
    print(f"  Monster moves: {len(monster_moves)}")
    print(f"  Combat events: {len(combats)}")
    print(f"  Item events: {len(items)}")

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

    if dungeons:
        seen_levels = set()
        print(f"\n{'='*20} DUNGEON LAYOUT {'='*20}")
        for d in dungeons:
            lvl = d.get('level', 0)
            if lvl not in seen_levels:
                seen_levels.add(lvl)
                print(f"  Level {lvl}: {d.get('rooms', '?')} rooms, {d.get('hallways', '?')} hallways")

    if show_map and dungeons:
        print(f"\n{'='*20} MAP SNAPSHOTS {'='*20}")
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

    print(f"\n{'='*20} ITEMS {'='*20}")
    if items:
        for item in items:
            action = item.get('action', '?')
            name = item.get('item', '?')
            pos = item.get('position', {})
            print(f"  [{action.upper():8}] {name} at ({pos.get('x')},{pos.get('y')})")
    else:
        print("  No item events logged")

    if dungeons:
        first_items = dungeons[0].get('items', [])
        if first_items:
            print(f"\n  Items visible at session start ({len(first_items)}):")
            for i in first_items[:10]:
                print(f"    - {i.get('name')} '{i.get('char')}' at ({i.get('x')},{i.get('y')})")
            if len(first_items) > 10:
                print(f"    ... and {len(first_items) - 10} more")

    print(f"\n{'='*20} MONSTERS {'='*20}")
    if dungeons:
        all_monsters = set()
        for d in dungeons:
            for m in d.get('monsters', []):
                all_monsters.add(m.get('name'))
        print(f"  Unique monster types seen: {len(all_monsters)}")
        for name in sorted(all_monsters):
            print(f"    - {name}")

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

        kills = [c for c in combats if c.get('killed')]
        if kills:
            print(f"\n  Monsters killed:")
            kill_counts = Counter(c.get('defender') for c in kills)
            for monster, count in kill_counts.most_common():
                print(f"    - {monster} x{count}")

        print(f"\n  Combat log:")
        for c in combats:
            hit = "HIT" if c.get('hit') else "MISS"
            attacker = c.get('attacker', '?')
            defender = c.get('defender', '?')
            dmg = c.get('damage', 0)
            hp = c.get('defender_hp', '?')
            killed = " [KILLED]" if c.get('killed') else ""
            print(f"    {attacker} -> {defender}: {hit} (dmg:{dmg}) HP->{hp}{killed}")
    else:
        print("  No combat this session")

    print(f"\n{'='*20} PLAYER STATUS {'='*20}")
    if dungeons:
        p_first = dungeons[0].get('player', {})
        p_last = dungeons[-1].get('player', {})
        print(f"  Start: HP {p_first.get('hp')}/{p_first.get('max_hp')} at ({p_first.get('x')},{p_first.get('y')})")
        print(f"  End:   HP {p_last.get('hp')}/{p_last.get('max_hp')} at ({p_last.get('x')},{p_last.get('y')})")

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
            for turn, diff, hp in hp_changes[-10:]:
                sign = "+" if diff > 0 else ""
                print(f"    Turn {turn}: {sign}{diff} -> HP {hp}")

    if player_moves:
        results = Counter(m.get('result') for m in player_moves)
        print(f"\n  Movement outcomes:")
        for r, c in results.most_common():
            print(f"    - {r}: {c}")

    print(f"\n{'='*20} OUTCOME {'='*20}")

    if deaths:
        death = deaths[-1]
        print(f"  DIED!")
        print(f"  Killed by: {death.get('killed_by', 'Unknown')}")
        print(f"  Level: {death.get('level', '?')}")
    elif dungeons:
        final_hp = dungeons[-1].get('player', {}).get('hp', 1)
        if final_hp <= 0:
            print(f"  DIED! (HP reached 0)")
        else:
            print(f"  Survived!")
    else:
        print(f"  Unknown outcome")

    level_changes = [e for e in events if e.get('type') == 'level_change']
    if level_changes:
        print(f"\n  Level changes:")
        for lc in level_changes:
            direction = lc.get('direction', 'unknown')
            from_lvl = lc.get('from_level', '?')
            to_lvl = lc.get('to_level', '?')
            print(f"    {direction} from L{from_lvl} to L{to_lvl}")

    if show_timeline:
        print(f"\n{'='*20} EVENT TIMELINE {'='*20}")
        for e in events[:50]:
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
