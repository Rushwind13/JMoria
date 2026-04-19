#!/usr/bin/env python3
"""
Analyze Items.txt to produce a gap analysis of item coverage by type.
Shows counts, magic/effect coverage, and notes on empty types.
"""

import re
import sys
from pathlib import Path
from collections import defaultdict

# Item type indices (from src/Constants.h)
ITEM_TYPES = [
    ("SWORD", 0),
    ("SHIELD", 1),
    ("ARMOR", 2),
    ("HELMET", 3),
    ("CLOAK", 4),
    ("GLOVES", 5),
    ("BOOTS", 6),
    ("AMULET", 7),
    ("RING", 8),
    ("TORCH", 9),
    ("BOW", 10),
    ("ARROW", 11),
    ("XBOW", 12),
    ("BOLT", 13),
    ("CHEST", 14),
    ("SCROLL", 15),
    ("POTION", 16),
    ("WAND", 17),
    ("STAFF", 18),
    ("BOOK", 19),
    ("MONEY", 20),
    ("FOOD", 21),
    ("SPEAR", 22),
    ("AXE", 23),
    ("POLEARM", 24),
    ("SHOVEL", 25),
    ("DAGGER", 26),
    ("MACE", 27),
    ("2H_SWORD", 28),
    ("BELT", 29),
    ("FUEL", 30),
]

# Map of item type to special notes
NOTES_MAP = {
    "ARMOR": "No magical armor",
    "HELMET": "Infravision, Telepathy",
    "TORCH": "Light intrinsic",
}


def parse_items_file(filepath):
    """Parse Items.txt and return list of item dicts."""
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
            'has_effect': False,
        }
        
        # Parse Type line
        type_match = re.search(r'Type\s*<ITEM_IDX_([A-Z0-9_]+)>', item_body)
        if type_match:
            item_dict['type'] = type_match.group(1)
        
        # Check for Effect line
        if re.search(r'Effect\s*<', item_body):
            item_dict['has_effect'] = True
        
        # Check for To-HitBonus, To-DamBonus, ACBonus, Damage (magical attributes)
        if (re.search(r'To-HitBonus\s*<', item_body) or
            re.search(r'To-DamBonus\s*<', item_body) or
            re.search(r'ACBonus\s*<', item_body)):
            item_dict['has_effect'] = True
        
        items.append(item_dict)
    
    return items


def analyze_items(items):
    """Analyze items and produce counts by type."""
    counts = defaultdict(int)
    magic_counts = defaultdict(int)
    
    for item in items:
        if item['type']:
            counts[item['type']] += 1
            if item['has_effect']:
                magic_counts[item['type']] += 1
    
    return counts, magic_counts


def main():
    # Find Items.txt relative to script location
    script_dir = Path(__file__).parent
    items_file = script_dir.parent / "Resources" / "Items.txt"
    
    if not items_file.exists():
        print(f"Error: {items_file} not found", file=sys.stderr)
        sys.exit(1)
    
    items = parse_items_file(str(items_file))
    counts, magic_counts = analyze_items(items)
    
    # Print header
    print("Gap Analysis: Items, Effects, and Coverage")
    print("Current Counts by ITEM_IDX")
    print("ITEM_IDX\tCount\tMagic\tNotes")
    
    total_items = 0
    total_magic = 0
    unique_effects = set()
    
    # Track which types we've seen
    seen_types = set()
    for item in items:
        if item['type']:
            seen_types.add(item['type'])
    
    # Print in order from ITEM_TYPES
    for type_name, type_idx in ITEM_TYPES:
        count = counts.get(type_name, 0)
        magic = magic_counts.get(type_name, 0)
        note = NOTES_MAP.get(type_name, "")
        
        if count == 0:
            note = "Empty type"
        
        total_items += count
        total_magic += magic
        
        print(f"{type_name}\t{count}\t{magic}\t{note}")
    
    # Count unique effects
    for item in items:
        if item['has_effect']:
            unique_effects.add(item['name'])
    
    print(f"Totals: {total_items} items, {total_magic} magical, {len(unique_effects)} named effects")


if __name__ == '__main__':
    main()
