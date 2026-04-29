#!/usr/bin/env python3
"""
One-time migration: apply level redesign to Items.txt (April 2026).
Run this once, then run sort_resources.py to re-sort.
"""

import re
import sys
from pathlib import Path

ITEMS_FILE = Path(__file__).parent.parent / "Resources" / "Items.txt"

# Maps exact item name (as it appears in  Item <Name>) to new Level value.
# Items NOT listed here are unchanged.
LEVEL_CHANGES = {
    # --- Swords ---
    "Long Sword":       14,
    "Bastard sword":    25,
    "Scimitar":         18,
    # --- Maces & Clubs ---
    "Club":              5,
    "Mace":             14,
    "Morning Star":     20,
    "War Hammer":       20,
    "Battle Mace":      28,
    # --- Spears & Lances ---
    "Lance":            20,
    "Spear":             8,
    # --- Polearms ---
    "Glaive":           22,
    "Halberd":          28,
    "Quarter Staff":     8,
    "Trident":          18,
    # --- Bows ---
    "Long Bow":         20,
    # --- Axes ---
    "Hand Axe":          8,
    "Battle Axe":       25,
    # --- Body Armor ---
    "Leather Armor":     5,
    # Padded Armor stays at 7 — no change
    "Splint Mail":      32,
    "Chain Mail":       38,
    "Plate Mail Armor": 52,
    # --- Shields ---
    "Small Wooden Shield":  5,
    "Large Steel Shield":  22,
    "Small Steel Shield":  14,
    "Steel Shield":        16,
    # --- Helmets ---
    "Steel Cap":        12,
    "Steel Helm":       18,
    "Helm of Infravision":         28,
    "Helm of Telepathy":           42,
    "Helmet of Lordly Protection": 65,
    # --- Cloaks ---
    "Cloak of Protection": 25,
    # --- Gloves ---
    "Set of Gauntlets":    16,
    # --- Boots ---
    "Pair of Steel Boots": 16,
    "Boots of Speed":      48,
    # --- Rings ---
    "Ring of Tunneling":             8,
    "Ring of Fire Resistance":      12,
    "Ring of Cold Resistance":      12,
    "Ring of Greed":                15,
    "Ring of Searching":            15,
    "Ring of Acid Resistance":      18,
    "Ring of Electricity Resistance": 18,
    "Ring of Free Action":          40,
    "Ring of Sensing":              25,
    "Ring of Invisibility":         32,
    "Ring of Levitation":           30,
    "Ring of Protection":           32,
    "Ring of Telepathy":            35,
    "Ring of Speed":                52,
    # --- Scrolls ---
    "Scroll of Identify":              10,
    "Scroll of Remove Curse":           5,
    "Scroll of Phase Door":             3,
    "Scroll of Detect Monsters":        5,
    "Scroll of Detect Traps":           5,
    "Scroll of Summon Monsters":       20,
    "Scroll of Treasure Detection":     5,
    "Scroll of Telepathy":              6,
    "Scroll of Blessing":               8,
    "Scroll of Curse Object":           8,
    "Scroll of Scare Monster":          8,
    "Scroll of Magic Mapping":         25,
    "Scroll of Recharging":            25,
    "Scroll of Word of Recall":        18,
    "Scroll of Enchant Armor":         22,
    "Scroll of Enchant Weapon Damage": 22,
    "Scroll of Enchant Weapon to Hit": 22,
    "Scroll of Mass Sleep":            25,
    "Scroll of Teleportation":         15,
    "Scroll of *Enchant Armor*":       52,
    "Scroll of *Enchant Weapon*":      52,
    "Scroll of *Recharging*":          52,
    "Scroll of *Magic Mapping*":       58,
    # --- Potions ---
    "Potion of Blindness":              4,
    "Potion of Confusion":              4,
    "Potion of Sleep":                  4,
    "Potion of Minor Healing":          3,
    "Potion of Slow Poison":            5,
    "Potion of Cure Poison":            8,
    "Potion of Flames":                 8,
    "Potion of Poison":                 8,
    "Potion of Infravision":           10,
    "Potion of Resist Cold":           10,
    "Potion of Resist Fire":           10,
    "Potion of Courage":                8,
    "Potion of Levitation":            15,
    "Potion of Neutralize Poison":     15,
    "Potion of Resist Acid":           15,
    "Potion of Resist Electricity":    15,
    "Potion of Invisibility":          22,
    "Potion of Speed":                 35,
    "Potion of Cure Light Wounds":     28,
    "Potion of Cure Serious Wounds":   48,
    "Potion of Cure Critical Wounds":  62,
    "Potion of Healing":               68,
    "Potion of *Healing*":             82,
    # --- Wands ---
    "Wand of Light":           3,
    "Wand of Stone to Mud":    6,
    "Wand of Sleep":           8,
    "Wand of Firebolts":      10,
    "Wand of Confusion":      10,
    "Wand of Fear":           12,
    "Wand of Frost":          16,
    "Wand of Lightning":      22,
    "Wand of Acid":           28,
    "Wand of Paralyze":       40,
    "Wand of Probing":        40,
    "Wand of Teleport Away":  25,
    "Wand of Summoning":      22,
    "Wand of Fireballs":      32,
    "Wand of Frost Balls":    36,
    "Wand of Lightning Balls": 40,
    "Wand of Acid Balls":     44,
    # --- Staves ---
    "Staff of Light":                  10,
    "Staff of Teleportation":          25,
    "Staff of Treasure Detection":     20,
    "Staff of Healing":                20,
    "Staff of Perception":             30,
    "Staff of Sleep":                  23,
    "Staff of Acid Resistance":        25,
    "Staff of Cold Resistance":        25,
    "Staff of Electricity Resistance": 25,
    "Staff of Fear":                   25,
    "Staff of Fire Resistance":        25,
    "Staff of Mapping":                20,
    "Staff of Protection":             25,
    "Staff of Starlight":              25,
    "Staff of Mass Sleep":             35,
    "Staff of Paralysis":              30,
    "Staff of Summoning":              30,
    "Staff of Telepathy":              30,
    "Staff of Word of Recall":         50,
    "Staff of Cure Light Wounds":       8,
    "Staff of Resistance":             40,
    "Staff of *Resistance*":           55,
    "Staff of Cure Serious Wounds":    55,
    "Staff of Greater Healing":        75,
}

# New items to append (sort_resources.py will place them in the right section)
NEW_ITEMS = """\
Item <Leather Cap>
{
    Plural      <Leather Caps>
    Type        <ITEM_IDX_HELMET>
    Value       8.0
    Level       1
    AC          1.0
    Weight      1.0
    Color       <139,90,43,255>
}

Item <Short Sword>
{
    Plural      <Short Swords>
    Type        <ITEM_IDX_SWORD>
    Value       80.0
    Level       5
    Damage      <1d6>
    Weight      3.0
    Speed       3.0
    Color       <200,200,200,255>
}

Item <Short Bow>
{
    Plural      <Short Bows>
    Type        <ITEM_IDX_BOW>
    Value       120.0
    Flags       <ITEM_FLAG_NEEDSAMMO>
    Level       3
    Damage      <1d5>
    Weight      3.0
    Speed       4.0
    Color       <150,100,50,255>
}

# [RESERVED: Potion of Gain Strength/Intelligence/Wisdom/Dexterity/Constitution — Level 60-70]
# [RESERVED: Potion of Restore Life Levels — Level 60]
# [RESERVED: Ego items (ITEM_FLAG_EGO) — Level 38-90]
# [RESERVED: Legendary items (ITEM_FLAG_LEGENDARY) — Level 55-100]
# [RESERVED: Unique items (ITEM_FLAG_UNIQUE) — Level 70-100]
"""

# This comment survives sort_resources.py (no # --- or em-dash patterns)
SPAWN_WINDOW_COMMENT = """\
#
# SPAWN WINDOW REDESIGN (TODO: implement bell-curve item placement in C++ engine)
#
# CURRENT BEHAVIOR: Level = hard floor. Item spawns in dungeon levels [Level, Level+10].
#
# DESIGN INTENT: Level should become the PEAK depth (center of a bell curve).
#   Items appear with peak probability at their Level, fade out above and below.
#   Out-of-depth items (shallower than Level) possible but rare.
#   Below-depth items (deeper than Level) possible but increasingly uncommon.
#   Some items (e.g. healing potions) remain findable at great depth with slow fade.
#
# Proposed new data fields (not yet in parser or engine):
#   LevelPeak   int    dungeon depth at which item is most common  (= current Level)
#   LevelSigma  float  spread of the bell curve (default ~10; consumables wider)
#   LevelFloor  int    hard minimum depth (0 = can trickle up to town)
#   LevelCeil   int    hard maximum depth (100 = never fades entirely)
#
# RESERVED LEVEL SPACE (items not yet implemented):
#   Level 60-70  Potion of Gain Str/Int/Wis/Dex/Con (x5)
#   Level 60     Potion of Restore Life Levels
#   Level 38-90  Ego items (ITEM_FLAG_EGO — not yet defined)
#   Level 55-100 Legendary items (ITEM_FLAG_LEGENDARY — not yet defined)
#   Level 70-100 Unique items (ITEM_FLAG_UNIQUE — not yet defined)
#
"""


def apply_level_changes(text, level_changes):
    """Replace Level field within each named Item block."""
    changed = []
    not_found = []

    for name, new_level in level_changes.items():
        # Match: Item <name> then { ... Level N ... }
        # Capture the whitespace preceding the digit so indentation is preserved
        pattern = (
            r'(Item\s+<' + re.escape(name) + r'>\s*\{[^}]*?'
            r'\bLevel\s+)\d+'
        )
        new_text, n = re.subn(pattern, r'\g<1>' + str(new_level), text, flags=re.DOTALL)
        if n == 0:
            not_found.append(name)
        else:
            changed.append(name)
            text = new_text

    if not_found:
        print("WARNING: could not find item(s):", not_found, file=sys.stderr)
    print(f"Changed Level for {len(changed)} items.", file=sys.stderr)
    return text


def insert_spawn_comment(text, comment):
    """Insert spawn-window comment into the file header (before first Item block)."""
    # Find the first Item block
    m = re.search(r'^Item\s+<', text, re.MULTILINE)
    if not m:
        return text + comment
    insert_pos = m.start()
    return text[:insert_pos] + comment + text[insert_pos:]


def main():
    text = ITEMS_FILE.read_text(encoding='utf-8')

    # 1. Apply level changes
    text = apply_level_changes(text, LEVEL_CHANGES)

    # 2. Insert spawn-window design comment into file header
    #    (only if not already present)
    if 'SPAWN WINDOW REDESIGN' not in text:
        text = insert_spawn_comment(text, SPAWN_WINDOW_COMMENT)

    # 3. Append new item entries
    text = text.rstrip('\n') + '\n\n' + NEW_ITEMS

    ITEMS_FILE.write_text(text, encoding='utf-8')
    print(f"Written to {ITEMS_FILE}", file=sys.stderr)
    print("Now run:  python3 util/sort_resources.py", file=sys.stderr)


if __name__ == '__main__':
    main()
