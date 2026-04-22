#!/usr/bin/env python3
"""
sort_resources.py - Sort Items.txt, Effects.txt, and Monsters.txt
by their primary type index, then Level, then name alphabetically.
Inserts section-boundary comments between type groups.
"""

import re

# ──────────────────────────────────────────────────────────────────────────────
# Ordered type lists (define sort order and section header labels)
# ──────────────────────────────────────────────────────────────────────────────

ITEM_IDX_ORDER = [
    ("ITEM_IDX_SWORD",    "One-Handed Swords"),
    ("ITEM_IDX_2H_SWORD", "Two-Handed Swords"),
    ("ITEM_IDX_DAGGER",   "Daggers"),
    ("ITEM_IDX_AXE",      "Axes"),
    ("ITEM_IDX_MACE",     "Maces & Clubs"),
    ("ITEM_IDX_SPEAR",    "Spears & Lances"),
    ("ITEM_IDX_POLEARM",  "Polearms"),
    ("ITEM_IDX_BOW",      "Bows"),
    ("ITEM_IDX_XBOW",     "Crossbows"),
    ("ITEM_IDX_ARROW",    "Arrows"),
    ("ITEM_IDX_BOLT",     "Bolts"),
    ("ITEM_IDX_SHOVEL",   "Tools (Shovels / Pickaxes)"),
    ("ITEM_IDX_ARMOR",    "Body Armor"),
    ("ITEM_IDX_SHIELD",   "Shields"),
    ("ITEM_IDX_HELMET",   "Helmets"),
    ("ITEM_IDX_CLOAK",    "Cloaks"),
    ("ITEM_IDX_GLOVES",   "Gloves & Gauntlets"),
    ("ITEM_IDX_BOOTS",    "Boots"),
    ("ITEM_IDX_BELT",     "Belts"),
    ("ITEM_IDX_AMULET",   "Amulets"),
    ("ITEM_IDX_RING",     "Rings"),
    ("ITEM_IDX_TORCH",    "Light Sources"),
    ("ITEM_IDX_FUEL",     "Fuel"),
    ("ITEM_IDX_SCROLL",   "Scrolls"),
    ("ITEM_IDX_POTION",   "Potions"),
    ("ITEM_IDX_WAND",     "Wands"),
    ("ITEM_IDX_STAFF",    "Staves"),
    ("ITEM_IDX_BOOK",     "Books"),
    ("ITEM_IDX_CHEST",    "Chests"),
    ("ITEM_IDX_FOOD",     "Food"),
    ("ITEM_IDX_MONEY",    "Money"),
]

MON_IDX_ORDER = [
    # lowercase-tile monsters (0-26)
    ("MON_IDX_ANT",              "ANTS (a)"),
    ("MON_IDX_BAT",              "BATS (b)"),
    ("MON_IDX_CENTIPEDE",        "CENTIPEDES (c)"),
    ("MON_IDX_DRAGON",           "DRAGONS (d)"),
    ("MON_IDX_HYDRA",            "HYDRAS (d) - shares tile with DRAGON"),
    ("MON_IDX_EYE",              "EYES (e)"),
    ("MON_IDX_FLY",              "FLIES (f)"),
    ("MON_IDX_DRAGON_FLY",       "DRAGON FLIES (f) - shares tile with FLY"),
    ("MON_IDX_FAERIE_DRAGON",    "FAERIE DRAGONS (f) - shares tile with FLY"),
    ("MON_IDX_GOLEM",            "GOLEMS (g)"),
    ("MON_IDX_HUMANOID",         "HUMANOIDS (h)"),
    ("MON_IDX_ICKY",             "ICKY THINGS (i)"),
    ("MON_IDX_OOZE",             "OOZES (j)"),
    ("MON_IDX_KOBOLD",           "KOBOLDS (k)"),
    ("MON_IDX_LOUSE",            "LICE (l)"),
    ("MON_IDX_LEECH",            "LEECHES (l) - shares tile with LOUSE"),
    ("MON_IDX_MOLD",             "MOLDS (m)"),
    ("MON_IDX_NAGA",             "NAGAS (n)"),
    ("MON_IDX_ORC",              "ORCS (o)"),
    ("MON_IDX_PERSON",           "PERSONS (p) - class spread"),
    ("MON_IDX_RAT",              "RATS (r)"),
    ("MON_IDX_SKELETON",         "SKELETONS (s)"),
    ("MON_IDX_TOWNSFOLK",        "TOWNSFOLK (t)"),
    ("MON_IDX_MINOR_DEMON",      "MINOR DEMONS (u)"),
    ("MON_IDX_WORM",             "WORMS (w)"),
    ("MON_IDX_SPIDER",           "SPIDERS (x)"),
    ("MON_IDX_YEEK",             "YEEKS (y)"),
    ("MON_IDX_ZOMBIE",           "ZOMBIES (z)"),
    # uppercase-tile monsters (27-54)
    ("MON_IDX_FROG",             "FROGS / SLAADI (A)"),
    ("MON_IDX_BALROG",           "BALROGS (B)"),
    ("MON_IDX_DOG",              "DOGS / HOUNDS (C)"),
    ("MON_IDX_ANCIENT_DRAGON",   "ANCIENT DRAGONS (D)"),
    ("MON_IDX_DINOSAUR",         "DINOSAURS (D) - shares tile with ANCIENT_DRAGON"),
    ("MON_IDX_ELEMENTAL",        "ELEMENTALS (E)"),
    ("MON_IDX_BIRD",             "BIRDS (F)"),
    ("MON_IDX_GHOUL",            "GHOULS (G)"),
    ("MON_IDX_GHOST",            "GHOSTS (G) - shares tile with GHOUL"),
    ("MON_IDX_HARPY",            "HARPIES (H)"),
    ("MON_IDX_INSECT",           "INSECTS (I)"),
    ("MON_IDX_JELLY",            "JELLIES (J)"),
    ("MON_IDX_BEETLE",           "BEETLES (K)"),
    ("MON_IDX_LICH",             "LICHES (L)"),
    ("MON_IDX_MAMMAL",           "MAMMALS (M)"),
    ("MON_IDX_CAT",              "CATS (M) - shares tile with MAMMAL"),
    ("MON_IDX_OGRE",             "OGRES (O)"),
    ("MON_IDX_GIANT",            "GIANTS (P)"),
    ("MON_IDX_L_PERSON",         "LEGENDARY PERSONS (P) - shares tile with GIANT"),
    ("MON_IDX_REPTILE",          "REPTILES (R)"),
    ("MON_IDX_SNAKE",            "SNAKES (S)"),
    ("MON_IDX_TROLL",            "TROLLS (T)"),
    ("MON_IDX_MAJOR_DEMON",      "MAJOR DEMONS (U)"),
    ("MON_IDX_VAMPIRE",          "VAMPIRES (V)"),
    ("MON_IDX_WIGHT",            "WIGHTS (W)"),
    ("MON_IDX_WRAITH",           "WRAITHS (W) - shares tile with WIGHT"),
    ("MON_IDX_XORN",             "XORN (X)"),
    ("MON_IDX_YETI",             "YETI (Y)"),
    # special tiles
    ("MON_IDX_MIMIC",            "MIMICS (&)"),
    ("MON_IDX_LURKER",           "LURKERS / TRAPPERS (.)"),
    ("MON_IDX_SHROOM",           "MUSHROOMS (,)"),
    ("MON_IDX_COIN",             "CREEPING COINS ($)"),
    ("MON_IDX_ANIMATED_WEAPON",  "ANIMATED WEAPONS (|)"),
]

# EFFECT_TYPE ordering (by constant value, ascending)
EFFECT_TYPE_ORDER = [
    ("EFFECT_TYPE_HIT",       "HIT - damage / status attacks"),
    ("EFFECT_TYPE_HEAL",      "HEAL - restore HP and cure status"),
    ("EFFECT_TYPE_CREATE",    "CREATE - summon, teleport, recall, mapping"),
    ("EFFECT_TYPE_DESTROY",   "DESTROY - remove curses, poison, etc."),
    ("EFFECT_TYPE_INTRINSIC", "INTRINSIC - permanent and timed intrinsics"),
    ("EFFECT_TYPE_RESTORE",   "RESTORE - identify, recharge"),
    ("EFFECT_TYPE_GAIN",      "GAIN - enchant weapons/armor"),
    ("EFFECT_TYPE_LOSE",      "LOSE - drain / de-enchant"),
    ("EFFECT_TYPE_SEE",       "SEE - detect and reveal"),
]

# Within EFFECT_TYPE_HIT, secondary sort by Flag value (bit order from Constants.h)
EFFECT_FLAG_ORDER = [
    "EFFECT_FLAG_FIRE",
    "EFFECT_FLAG_COLD",
    "EFFECT_FLAG_ELECTRICITY",
    "EFFECT_FLAG_ACID",
    "EFFECT_FLAG_POISON",
    "EFFECT_FLAG_LIGHT",
    "EFFECT_FLAG_PARALYZE",
    "EFFECT_FLAG_TREASURE",
    "EFFECT_FLAG_AFRAID",
    "EFFECT_FLAG_BLIND",
    "EFFECT_FLAG_SLEEP",
    "EFFECT_FLAG_CONFUSE",
    "EFFECT_FLAG_STONE_TO_MUD",
    "EFFECT_FLAG_FUEL",
    "EFFECT_FLAG_INFRA",
    "EFFECT_FLAG_ESP",
    "EFFECT_FLAG_IDENTIFY",
    "EFFECT_FLAG_RECALL",
    "EFFECT_FLAG_MAPPING",
    "EFFECT_FLAG_SUMMON",
    "EFFECT_FLAG_STAT",
    "EFFECT_FLAG_TOHIT",
    "EFFECT_FLAG_TODAM",
    "EFFECT_FLAG_AC",
    "EFFECT_FLAG_XP",
    "EFFECT_FLAG_HP",
    "EFFECT_FLAG_MP",
    "EFFECT_FLAG_TELEPORT",
    "EFFECT_FLAG_FREE_ACTION",
    "EFFECT_FLAG_INVISIBLE",
    "EFFECT_FLAG_LEVITATE",
    "EFFECT_FLAG_SPEED",
    # flag2 namespace
    "EFFECT_FLAG_DOOR",
    "EFFECT_FLAG_TRAP",
    "EFFECT_FLAG_MONSTERS",
    "EFFECT_FLAG_NO_COLLIDE",
    "EFFECT_FLAG_CURSE",
]

# ──────────────────────────────────────────────────────────────────────────────
# Generic block parser
# ──────────────────────────────────────────────────────────────────────────────

def parse_blocks(text):
    """
    Parse the resource file into a list of blocks.
    Each block is a dict:
      {
        'kind':    'item' | 'monster' | 'effect' | 'comment' | 'blank',
        'name':    str (for item/monster/effect),
        'lines':   [str, ...],       # raw lines (no trailing newline) of the block
        'type':    str,              # value of Type field
        'flag':    str,              # value of Flag field (effects only)
        'level':   int,              # value of Level field (0 if absent)
      }
    Comment blocks: contiguous # lines or blank lines before the next keyword.
    We attach leading comments to the block that follows them.
    """
    lines = text.splitlines()
    blocks = []
    i = 0
    n = len(lines)

    while i < n:
        line = lines[i]
        stripped = line.strip()

        # keyword: Item / Monster / Effect
        m = re.match(r'^(Item|Monster|Effect)\s+<(.+)>', stripped)
        if m:
            keyword = m.group(1).lower()
            name = m.group(2)
            block_lines = [lines[i]]
            i += 1
            # consume until closing brace
            depth = 0
            while i < n:
                block_lines.append(lines[i])
                if '{' in lines[i]:
                    depth += 1
                if '}' in lines[i]:
                    depth -= 1
                    if depth <= 0:
                        i += 1
                        break
                i += 1
            body = '\n'.join(block_lines)
            # extract fields - anchor to line start so MoveType doesn't match
            type_m = re.search(r'^\s*Type\s+<([^>]+)>', body, re.MULTILINE)
            flag_m = re.search(r'^\s+Flag\s+<([^>]+)>', body, re.MULTILINE)
            level_m = re.search(r'Level\s+(\d+)', body)
            blocks.append({
                'kind':  keyword,
                'name':  name,
                'lines': block_lines,
                'type':  type_m.group(1) if type_m else '',
                'flag':  flag_m.group(1) if flag_m else '',
                'level': int(level_m.group(1)) if level_m else 0,
            })
        else:
            # accumulate as raw (comment/blank) lines
            block_lines = [lines[i]]
            i += 1
            # merge contiguous non-keyword lines
            while i < n:
                next_stripped = lines[i].strip()
                if re.match(r'^(Item|Monster|Effect)\s+<', next_stripped):
                    break
                block_lines.append(lines[i])
                i += 1
            blocks.append({
                'kind':  'raw',
                'lines': block_lines,
            })

    return blocks


def render_block(b):
    return '\n'.join(b['lines'])


# ──────────────────────────────────────────────────────────────────────────────
# Sort helpers
# ──────────────────────────────────────────────────────────────────────────────

def make_type_rank(order_list):
    return {typ: i for i, (typ, _) in enumerate(order_list)}

def sort_key_item(b):
    rank = make_type_rank(ITEM_IDX_ORDER)
    return (rank.get(b['type'], 999), b['level'], b['name'].lower())

def sort_key_monster(b):
    rank = make_type_rank(MON_IDX_ORDER)
    return (rank.get(b['type'], 999), b['level'], b['name'].lower())

def sort_key_effect(b):
    type_rank = make_type_rank(EFFECT_TYPE_ORDER)
    flag_rank = {f: i for i, f in enumerate(EFFECT_FLAG_ORDER)}
    return (type_rank.get(b['type'], 999), flag_rank.get(b['flag'], 999), b['name'].lower())


# ──────────────────────────────────────────────────────────────────────────────
# File-header extractor (everything before the first keyword block)
# ──────────────────────────────────────────────────────────────────────────────

# All section labels ever emitted by this script - used to strip stale lines
_ALL_SECTION_LABELS = (
    {label for _, label in ITEM_IDX_ORDER}
    | {label for _, label in MON_IDX_ORDER}
    | {label for _, label in EFFECT_TYPE_ORDER}
)

def extract_header_and_entries(blocks):
    header = []
    entries = []
    in_header = True
    for b in blocks:
        if in_header and b['kind'] == 'raw':
            # Strip stale section-boundary lines left by a previous sort run
            clean = []
            for l in b['lines']:
                s = l.strip()
                # Generated boundary: # --- ... --- or # ═══...
                if re.match(r'^#\s*(---|\u2550)', s):
                    continue
                # Bare type-label line:  # HIT - damage / status attacks
                m = re.match(r'^#\s+(.+)', s)
                if m and m.group(1) in _ALL_SECTION_LABELS:
                    continue
                # Orphaned per-entry comment (contains em-dash - not used in the format spec)
                if '\u2014' in s and s.startswith('#'):
                    continue
                # Orphaned "Used by:" comment line
                if re.match(r'^#\s+Used by:', s):
                    continue
                clean.append(l)
            # Collapse runs of blank lines to at most one; strip trailing blanks
            collapsed = []
            prev_blank = False
            for l in clean:
                is_blank = l.strip() == ''
                if is_blank and prev_blank:
                    continue
                collapsed.append(l)
                prev_blank = is_blank
            # strip trailing blank lines
            while collapsed and collapsed[-1].strip() == '':
                collapsed.pop()
            if collapsed:
                b = dict(b, lines=collapsed)
                header.append(b)
        else:
            in_header = False
            if b['kind'] != 'raw':
                entries.append(b)
            # silently discard raw blocks inside the data section
    return header, entries


# ──────────────────────────────────────────────────────────────────────────────
# Rebuild file with section banners
# ──────────────────────────────────────────────────────────────────────────────

def rebuild_with_sections(header_blocks, sorted_entries, order_list, kind_name):
    out_parts = []

    # header
    for b in header_blocks:
        out_parts.append(render_block(b))

    type_rank = {typ: i for i, (typ, _) in enumerate(order_list)}
    type_labels = {typ: label for typ, label in order_list}

    current_type = None
    for b in sorted_entries:
        btype = b['type']
        if btype != current_type:
            current_type = btype
            label = type_labels.get(btype, btype)
            out_parts.append('')
            out_parts.append(f'# --- {label} ---')
            out_parts.append('')
        out_parts.append(render_block(b))

    return '\n'.join(out_parts) + '\n'


def rebuild_effects_with_sections(header_blocks, sorted_entries):
    out_parts = []
    for b in header_blocks:
        out_parts.append(render_block(b))

    type_labels = {typ: label for typ, label in EFFECT_TYPE_ORDER}
    current_type = None
    current_flag = None
    for b in sorted_entries:
        btype = b['type']
        bflag = b['flag']
        if btype != current_type:
            current_type = btype
            current_flag = None
            label = type_labels.get(btype, btype)
            out_parts.append('')
            out_parts.append(f'# --- {label} ---')
            out_parts.append('')
        # Only emit flag sub-headers under HIT (most flag-varied type)
        if btype == 'EFFECT_TYPE_HIT' and bflag and bflag != current_flag:
            current_flag = bflag
            out_parts.append(f'# --- {bflag} ---')
            out_parts.append('')
        out_parts.append(render_block(b))

    return '\n'.join(out_parts) + '\n'


# ──────────────────────────────────────────────────────────────────────────────
# Main
# ──────────────────────────────────────────────────────────────────────────────

def sort_items(path):
    text = open(path).read()
    blocks = parse_blocks(text)
    header, entries = extract_header_and_entries(blocks)
    entries.sort(key=sort_key_item)
    result = rebuild_with_sections(header, entries, ITEM_IDX_ORDER, 'item')
    open(path, 'w').write(result)
    print(f'Sorted {len(entries)} items → {path}')

def sort_monsters(path):
    text = open(path).read()
    blocks = parse_blocks(text)
    header, entries = extract_header_and_entries(blocks)
    entries.sort(key=sort_key_monster)
    result = rebuild_with_sections(header, entries, MON_IDX_ORDER, 'monster')
    open(path, 'w').write(result)
    print(f'Sorted {len(entries)} monsters → {path}')

def sort_effects(path):
    text = open(path).read()
    blocks = parse_blocks(text)
    header, entries = extract_header_and_entries(blocks)
    entries.sort(key=sort_key_effect)
    result = rebuild_effects_with_sections(header, entries)
    open(path, 'w').write(result)
    print(f'Sorted {len(entries)} effects → {path}')


if __name__ == '__main__':
    base = '/Users/jimbo/Documents/code/JMoria/Resources'
    sort_items(f'{base}/Items.txt')
    sort_monsters(f'{base}/Monsters.txt')
    sort_effects(f'{base}/Effects.txt')
    print('Done.')
