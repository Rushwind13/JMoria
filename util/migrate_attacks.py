#!/usr/bin/env python3
"""
migrate_attacks.py — Phase 3 Monsters.txt attack migration

Converts old inline Attack format:
    Attack  <EFFECT_TYPE_HIT>,<MON_FLAG_BITE>,2d8
    Attack  <EFFECT_TYPE_INTRINSIC>,<EFFECT_FLAG_CONFUSE>,3d8

To new named-effect format:
    Attack  <MON_FLAG_BITE>, <Physical Hit>, 2d8
    Attack  <MON_FLAG_TOUCH>, <Confusion Touch>, 3d8

Rules applied (in priority order):
  1. Breathe + element → bolt or ball depending on monster tier
  2. HIT + delivery + FIRE/COLD/ELECTRICITY/ACID → elemental Touch
  3. HIT + delivery + POISON → Poison Sting
  4. HIT + delivery + XP → XP Drain Touch (delivery preserved)
  5. INTRINSIC + PARALYZE/CONFUSE/BLIND/SLEEP/AFRAID → Touch effect
  6. INTRINSIC + POISON → Poison Touch (dice = duration)
  7. INTRINSIC + XP → MON_FLAG_BITE, XP Drain Touch
  8. INTRINSIC + TREASURE (Creeping Coins) → MON_FLAG_CRAWL, Poison Touch
  9. HIT + delivery (pure physical) → Physical Hit
  LEAVE AS-IS: STAT drain, Tourist malformed (EFFECT_FLAG_INTRINSIC prefix), 0d0 attacks
"""

import re
import sys

# Monsters that use BALL effects for breathe attacks
BALL_MONSTERS = {
    # Mature (unprefixed color) dragons on 'd' tile
    "White Dragon",
    "Black Dragon",
    "Green Dragon",
    "Blue Dragon",
    "Red Dragon",
    # Ancient dragons on 'D' tile
    "Ancient White Dragon",
    "Ancient Black Dragon",
    "Ancient Green Dragon",
    "Ancient Blue Dragon",
    "Ancient Red Dragon",
    "Ancient Multi-hued Dragon",
    # Major creatures
    "Balrog",
    "Frost Giant",
    "Fire Giant",
    "Cloud Giant",
    "Storm Giant",
    "Demon Lord",
    "Greater Demon Lord",
    "Titan",
    "Guardian Naga",
}

# Element → (bolt_effect, ball_effect)
ELEMENT_BREATHE = {
    "EFFECT_FLAG_FIRE":        ("Firebolt",       "Fireball"),
    "EFFECT_FLAG_COLD":        ("Frost Bolt",     "Frost Ball"),
    "EFFECT_FLAG_ELECTRICITY": ("Lightning Bolt", "Lightning Ball"),
    "EFFECT_FLAG_ACID":        ("Acid Bolt",      "Acid Ball"),
    "EFFECT_FLAG_POISON":      ("Poison Bolt",    "Poison Ball"),
}

# Element/status → elemental melee touch effect name (used in HIT path with delivery)
ELEMENT_TOUCH = {
    "EFFECT_FLAG_FIRE":        "Fire Touch",
    "EFFECT_FLAG_COLD":        "Cold Touch",
    "EFFECT_FLAG_ELECTRICITY": "Lightning Touch",
    "EFFECT_FLAG_ACID":        "Acid Touch",
    "EFFECT_FLAG_LIGHT":       "Light Touch",
    "EFFECT_FLAG_AFRAID":      "Fear Touch",
}

# INTRINSIC flag → melee touch effect name
INTRINSIC_STATUS = {
    "EFFECT_FLAG_PARALYZE": "Paralysis Touch",
    "EFFECT_FLAG_CONFUSE":  "Confusion Touch",
    "EFFECT_FLAG_BLIND":    "Blindness Touch",
    "EFFECT_FLAG_SLEEP":    "Sleep Touch",
    "EFFECT_FLAG_AFRAID":   "Fear Touch",
}

# Delivery tokens (kept verbatim in output)
DELIVERIES = {
    "MON_FLAG_BITE", "MON_FLAG_CLAW", "MON_FLAG_TOUCH",
    "MON_FLAG_TRAMPLE", "MON_FLAG_SPORE", "MON_FLAG_CRAWL",
    "MON_FLAG_BREATHE",
}

# Regex to split a token like <FOO> → just FOO (no angle brackets)
def strip_angle(s):
    return s.strip().lstrip("<").rstrip(">")

def leading_whitespace(line):
    return len(line) - len(line.lstrip())

def migrate_attack_line(line, current_monster):
    """Return the migrated line, or None to leave unchanged."""
    stripped = line.strip()
    if not stripped.startswith("Attack"):
        return None

    # Extract everything after 'Attack' keyword
    m = re.match(r'^(\s*Attack\s+)(.*)', line)
    if not m:
        return None
    prefix_raw = m.group(1)  # original whitespace+Attack+whitespace
    rest = m.group(2).rstrip()

    # Parse tokens: angle-bracketed and bare strings
    tokens = re.findall(r'<[^>]+>|[^\s,<>]+', rest)
    if not tokens:
        return None

    clean = [strip_angle(t) for t in tokens]

    # --- LEAVE AS-IS cases ---
    # Tourist malformed: starts with EFFECT_FLAG_INTRINSIC (not EFFECT_TYPE_)
    if clean[0] == "EFFECT_FLAG_INTRINSIC":
        return None
    # 0d0 attacks
    if any(re.match(r'0d0', t, re.IGNORECASE) for t in clean):
        return None
    # Already migrated (starts with MON_FLAG_)
    if clean[0].startswith("MON_FLAG_"):
        return None

    # From here, tokens[0] should be EFFECT_TYPE_HIT or EFFECT_TYPE_INTRINSIC
    etype = clean[0]

    # Standard indent for output (4 spaces + "Attack" + spaces to column 20)
    indent = "    "
    kw = "Attack"
    # Use consistent padding: "    Attack         " (19 chars for keyword+pad)
    atk_prefix = f"{indent}{kw:<19}"

    # -----------------------------------------------------------------------
    # BREATHE attacks
    # -----------------------------------------------------------------------
    if etype == "EFFECT_TYPE_HIT" and len(clean) >= 3 and clean[1] == "MON_FLAG_BREATHE":
        element = clean[2] if len(clean) > 2 else None
        dice = clean[3] if len(clean) > 3 else None
        rng  = clean[4] if len(clean) > 4 else None

        if element in ELEMENT_BREATHE:
            use_ball = (current_monster in BALL_MONSTERS)
            effect_name = ELEMENT_BREATHE[element][1 if use_ball else 0]
            parts = [f"<MON_FLAG_BREATHE>", f"<{effect_name}>"]
            if dice:
                parts.append(dice)
            if rng:
                parts.append(rng)
            return f"{atk_prefix}{', '.join(parts)}\n"

    # -----------------------------------------------------------------------
    # HIT attacks with a DELIVERY token
    # -----------------------------------------------------------------------
    if etype == "EFFECT_TYPE_HIT" and len(clean) >= 2:
        delivery = clean[1]
        if delivery not in DELIVERIES:
            return None  # unknown delivery — leave as-is

        flag    = clean[2] if len(clean) > 2 else None
        dice    = None
        rest_parts = clean[2:]  # everything after delivery

        # Separate flag tokens from dice/range tokens
        flags = []
        tail  = []
        for tok in rest_parts:
            if tok.startswith("EFFECT_FLAG_") or tok.startswith("MON_FLAG_"):
                flags.append(tok)
            else:
                tail.append(tok)  # dice / range

        dice_token  = tail[0] if len(tail) > 0 else None
        range_token = tail[1] if len(tail) > 1 else None

        # --- LEAVE STAT drains ---
        if "EFFECT_FLAG_STAT" in flags:
            return None

        # XP drain via HIT+XP
        if "EFFECT_FLAG_XP" in flags:
            parts = [f"<{delivery}>", "<XP Drain Touch>"]
            if dice_token: parts.append(dice_token)
            if range_token: parts.append(range_token)
            return f"{atk_prefix}{', '.join(parts)}\n"

        # Elemental melee (non-breathe)
        for eflag, touch_name in ELEMENT_TOUCH.items():
            if eflag in flags:
                parts = [f"<{delivery}>", f"<{touch_name}>"]
                if dice_token: parts.append(dice_token)
                return f"{atk_prefix}{', '.join(parts)}\n"

        # Poison melee via HIT
        if "EFFECT_FLAG_POISON" in flags:
            parts = [f"<{delivery}>", "<Poison Sting>"]
            if dice_token: parts.append(dice_token)
            return f"{atk_prefix}{', '.join(parts)}\n"

        # Pure physical (no flags, or flags we don't recognize → leave)
        if not flags:
            parts = [f"<{delivery}>", "<Physical Hit>"]
            if dice_token: parts.append(dice_token)
            return f"{atk_prefix}{', '.join(parts)}\n"

    # -----------------------------------------------------------------------
    # INTRINSIC attacks
    # -----------------------------------------------------------------------
    if etype == "EFFECT_TYPE_INTRINSIC":
        iflag = clean[1] if len(clean) > 1 else None
        dice  = clean[2] if len(clean) > 2 else None

        # Stat drain → leave as-is
        if iflag == "EFFECT_FLAG_STAT":
            return None

        # Status conditions
        if iflag in INTRINSIC_STATUS:
            effect_name = INTRINSIC_STATUS[iflag]
            parts = ["<MON_FLAG_TOUCH>", f"<{effect_name}>"]
            if dice: parts.append(dice)
            return f"{atk_prefix}{', '.join(parts)}\n"

        # Poison
        if iflag == "EFFECT_FLAG_POISON":
            parts = ["<MON_FLAG_TOUCH>", "<Poison Touch>"]
            if dice: parts.append(dice)
            return f"{atk_prefix}{', '.join(parts)}\n"

        # XP drain (Vampire Bat style)
        if iflag == "EFFECT_FLAG_XP":
            parts = ["<MON_FLAG_BITE>", "<XP Drain Touch>"]
            if dice: parts.append(dice)
            return f"{atk_prefix}{', '.join(parts)}\n"

        # TREASURE (Creeping Coins) → Poison Touch (crawl flavor)
        if iflag == "EFFECT_FLAG_TREASURE":
            return f"{atk_prefix}<MON_FLAG_CRAWL>, <Poison Touch>\n"

    return None  # leave unchanged


def migrate_file(path):
    with open(path) as f:
        lines = f.readlines()

    out = []
    current_monster = ""
    changed = 0
    unchanged_attacks = []

    for i, line in enumerate(lines):
        # Track current monster name
        mm = re.match(r'^(?:Monster|NPC)\s+<([^>]+)>', line.strip())
        if mm:
            current_monster = mm.group(1)

        result = migrate_attack_line(line, current_monster)
        if result is not None:
            out.append(result)
            changed += 1
        else:
            out.append(line)
            # Log any un-migrated attack lines (excluding comments and already-migrated)
            stripped = line.strip()
            if (stripped.startswith("Attack") and
                    not stripped.startswith("#") and
                    "EFFECT_TYPE_" in stripped and
                    "EFFECT_FLAG_STAT" not in stripped and
                    "EFFECT_FLAG_INTRINSIC" not in stripped and
                    "0d0" not in stripped):
                unchanged_attacks.append((i+1, current_monster, stripped))

    with open(path, "w") as f:
        f.writelines(out)

    print(f"Migrated {changed} Attack lines.")
    if unchanged_attacks:
        print(f"\nWARNING: {len(unchanged_attacks)} Attack lines NOT migrated:")
        for lineno, mon, text in unchanged_attacks:
            print(f"  line {lineno} [{mon}]: {text}")
    else:
        print("All legacy Attack lines migrated successfully.")

    return changed


if __name__ == "__main__":
    path = sys.argv[1] if len(sys.argv) > 1 else "Resources/Monsters.txt"
    migrate_file(path)
