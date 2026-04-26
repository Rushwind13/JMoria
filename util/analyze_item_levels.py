#!/usr/bin/env python3
"""
Analyze Items.txt to show item level distribution across dungeon levels 1-100.
Identifies level gaps and coverage patterns.
"""

import re
import sys
from pathlib import Path
from collections import defaultdict


def parse_items_file(filepath):
    """Parse Items.txt and return list of item dicts with level info."""
    items = []
    
    with open(filepath, 'r') as f:
        content = f.read()
    
    # Split by Item blocks
    item_pattern = r'Item\s+<([^>]+)>\s*\{([^}]+)\}'
    
    for match in re.finditer(item_pattern, content, re.MULTILINE):
        item_name = match.group(1)
        item_body = match.group(2)
        
        item_dict = {
            'name': item_name,
            'type': None,
            'level': None,
        }
        
        # Parse Type line
        type_match = re.search(r'Type\s*<ITEM_IDX_([A-Z0-9_]+)>', item_body)
        if type_match:
            item_dict['type'] = type_match.group(1)
        
        # Parse Level line
        level_match = re.search(r'Level\s+(\d+)', item_body)
        if level_match:
            item_dict['level'] = int(level_match.group(1))
        
        items.append(item_dict)
    
    return items


def analyze_level_distribution(items):
    """Analyze item distribution across levels."""
    level_counts = defaultdict(int)
    level_types = defaultdict(set)
    
    for item in items:
        if item['level'] is not None:
            level_counts[item['level']] += 1
            if item['type']:
                level_types[item['level']].add(item['type'])
    
    return level_counts, level_types


def main():
    # Find Items.txt relative to script location
    script_dir = Path(__file__).parent
    items_file = script_dir.parent / "Resources" / "Items.txt"
    
    if not items_file.exists():
        print(f"Error: {items_file} not found", file=sys.stderr)
        sys.exit(1)
    
    items = parse_items_file(str(items_file))
    level_counts, level_types = analyze_level_distribution(items)
    
    # Print header
    print("Item Level Distribution Across Dungeon (Levels 1-100)")
    print("=" * 70)
    print("Level\tCount\tTypes")
    
    # Track statistics
    levels_with_items = 0
    levels_without_items = 0
    total_items = 0
    min_level_with_items = 101
    max_level_with_items = 0
    
    # Print for levels 0-100
    for level in range(0, 101):
        count = level_counts.get(level, 0)
        types = level_types.get(level, set())
        type_str = ", ".join(sorted(types)) if types else ""
        
        if count == 0:
            levels_without_items += 1
            # Only print some empty levels to avoid clutter
            if level == 0 or (level > 0 and level % 10 == 0):
                print(f"{level}\t{count}\t-")
        else:
            levels_with_items += 1
            total_items += count
            min_level_with_items = min(min_level_with_items, level)
            max_level_with_items = max(max_level_with_items, level)
            print(f"{level}\t{count}\t{type_str}")
    
    # Print summary
    print("=" * 70)
    print(f"Summary:")
    print(f"  Levels with items: {levels_with_items}")
    print(f"  Levels without items: {levels_without_items}")
    print(f"  Total items placed: {total_items}")
    print(f"  Min level: {min_level_with_items if min_level_with_items <= 100 else 'N/A'}")
    print(f"  Max level: {max_level_with_items if max_level_with_items >= 1 else 'N/A'}")
    
    # Find gaps (ranges of consecutive levels with no items)
    print("\nLevel Gaps (no items available):")
    in_gap = False
    gap_start = 0
    
    for level in range(0, 101):
        has_items = level_counts.get(level, 0) > 0
        
        if not has_items and not in_gap:
            gap_start = level
            in_gap = True
        elif has_items and in_gap:
            if level - gap_start > 1:
                print(f"  Levels {gap_start}-{level-1}: {level-gap_start} levels with no items")
            in_gap = False


if __name__ == '__main__':
    main()
