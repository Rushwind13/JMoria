"""
screen.py — Read and parse the tmux pane output from jmoria (ASCII renderer).

Layout at 125×40 (ASCIILayout::CreateForSize from RenderASCII.cpp):
  MSG_HEIGHT  = 5   → rows 0–4
  STATS_WIDTH = 25  → cols 0–24, rows 5–39
  INV_WIDTH   = 25  → cols 100–124 (shown permanently when termW >= 100)
  DUNGEON     = cols 25–99, rows 5–39

Stats sidebar text labels (from Player::DisplayStats):
  "AC: N"
  "HP: N / N"
  "Level: N"
"""

import re
import subprocess
from pathlib import Path

from .state import GameState

SESSION = "crawler"

# Terminal dimensions we launch with (fallbacks; actual size read from tmux)
TERM_W = 125
TERM_H = 40

# Tracks last-detected pane width so helpers can use it without extra args
_last_term_w = 0

# Layout constants mirroring ASCIILayout::CreateForSize
MSG_HEIGHT = 5
STATS_WIDTH = 25
INV_WIDTH = 25
INV_AUTO_WIDTH = 100

# Fallback glyph sets, used if canonical source parsing fails.
_FALLBACK_MONSTER_CHARS = set("abcddefghhikllmnoprsuwxyzABCDFFFGGHIJKLOPRSTUVWWXY&.,$t")
_FALLBACK_ITEM_CHARS = set(r'|)[](]]"=~{}{}&?!-_?$~//\\/|/|]!')


def _load_id_chars(cpp_file: str, symbol: str, fallback: set[str]) -> set[str]:
    """Load canonical glyph mappings from C++ arrays (e.g., MonIDs/ItemIDs)."""
    try:
        root = Path(__file__).resolve().parents[2]
        text = (root / cpp_file).read_text(encoding="utf-8")
        pattern = rf'unsigned\s+char\s+{symbol}\[[^\]]+\]\s*=\s*"((?:[^"\\]|\\.)*)";'
        m = re.search(pattern, text)
        if not m:
            return fallback
        raw = m.group(1)
        decoded = bytes(raw, "utf-8").decode("unicode_escape")
        return set(decoded) if decoded else fallback
    except Exception:
        return fallback


_MONSTER_CHARS = _load_id_chars("src/Monster.cpp", "MonIDs", _FALLBACK_MONSTER_CHARS)
_ITEM_CHARS = _load_id_chars("src/Item.cpp", "ItemIDs", _FALLBACK_ITEM_CHARS)

# In ASCII mode, some monster IDs overlap terrain glyphs (e.g., '.', ',').
# Keep only unambiguous symbols for tactical decisions.
_MONSTER_CHARS_DISAMBIGUATED = {ch for ch in _MONSTER_CHARS if ch.isalpha() or ch == "&"}

# Glyphs that are terrain or UI framing, not entity markers.
_ENTITY_EXCLUDE_CHARS = set(" @#.:+'<>|lmxqtuvwj")

# Last parser stats for debugging parser quality in verbose bot logs.
_LAST_PARSE_DEBUG = {
    "accepted_monsters": 0,
    "accepted_items": 0,
    "rejected_monsters": 0,
    "rejected_items": 0,
}


def _is_word_like(row: list[str], col: int) -> bool:
    """True when this position appears inside plain text rather than map glyphs."""
    left_alpha = col > 0 and row[col - 1].isalpha()
    right_alpha = col + 1 < len(row) and row[col + 1].isalpha()
    return left_alpha or right_alpha


def _is_text_adjacent(row: list[str], col: int) -> bool:
    """Reject punctuation that appears adjacent to words in overlays/popups."""
    left_alnum = col > 0 and row[col - 1].isalnum()
    right_alnum = col + 1 < len(row) and row[col + 1].isalnum()
    return left_alnum or right_alnum


def _get_pane_size() -> tuple[int, int]:
    """Query the actual tmux pane dimensions (width, height)."""
    result = subprocess.run(
        ["tmux", "display-message", "-t", SESSION, "-p", "#{pane_width}x#{pane_height}"],
        check=True,
        capture_output=True,
        text=True,
    )
    w, h = result.stdout.strip().split("x")
    return int(w), int(h)


def _get_lines() -> tuple[list[str], int, int]:
    """Capture the tmux pane and return (lines, actual_width, actual_height)."""
    pane_w, pane_h = _get_pane_size()
    result = subprocess.run(
        ["tmux", "capture-pane", "-t", SESSION, "-p"],
        check=True,
        capture_output=True,
        text=True,
    )
    lines = result.stdout.splitlines()
    # Pad to actual pane height, each padded to actual pane width
    while len(lines) < pane_h:
        lines.append("")
    lines = lines[:pane_h]
    lines = [line.ljust(pane_w) for line in lines]
    return lines, pane_w, pane_h


def _strip_box(text: str) -> str:
    """Strip ncurses box-drawing border chars from a string."""
    return text.lstrip("lmxqtuvwj+|").rstrip("lmxqktuvwj+|")


def _parse_stats(lines: list[str], term_w: int) -> dict:
    """Parse values from the left stats panel (cols 0–STATS_WIDTH, rows MSG_HEIGHT+)."""
    # Strip the leading 'x' border char and trailing border chars from each row
    stats_text = "\n".join(
        _strip_box(line[:STATS_WIDTH]) for line in lines[MSG_HEIGHT:]
    )

    hp, max_hp, ac, level, depth_ft, world_pos = 0, 0, 0, 0, None, None

    m = re.search(r"HP:\s*(\d+)\s*/\s*(\d+)", stats_text)
    if m:
        hp, max_hp = int(m.group(1)), int(m.group(2))

    m = re.search(r"AC:\s*(\d+)", stats_text)
    if m:
        ac = int(m.group(1))

    m = re.search(r"Level:\s*(\d+)", stats_text)
    if m:
        level = int(m.group(1))

    m = re.search(r"Depth:\s*(\d+)'", stats_text)
    if m:
        depth_ft = int(m.group(1))

    m = re.search(r"Pos:\s*<\s*(-?\d+)\s+(-?\d+)\s*>", stats_text)
    if m:
        world_pos = (int(m.group(1)), int(m.group(2)))

    damage_dice = ""
    m = re.search(r"Damage:\s*(\d+d\d+)", stats_text)
    if m:
        damage_dice = m.group(1)

    to_hit_bonus = 0
    m = re.search(r"\+to Hit:\s*(-?\d+)", stats_text)
    if m:
        to_hit_bonus = int(m.group(1))

    to_dam_bonus = 0
    m = re.search(r"\+to Dam:\s*(-?\d+)", stats_text)
    if m:
        to_dam_bonus = int(m.group(1))

    return {
        "hp": hp,
        "max_hp": max_hp,
        "ac": ac,
        "level": level,
        "depth_ft": depth_ft,
        "world_pos": world_pos,
        "damage_dice": damage_dice,
        "to_hit_bonus": to_hit_bonus,
        "to_dam_bonus": to_dam_bonus,
    }


def get_panel_visibility(lines: list[str], term_w: int = 0) -> dict:
    """Detect whether key UI panels are currently visible in the ASCII layout."""
    tw = term_w or _last_term_w or TERM_W
    body_rows = lines[MSG_HEIGHT:]
    left_text = "\n".join(_strip_box(r[:STATS_WIDTH]) for r in body_rows)

    right_text = ""
    if tw >= INV_AUTO_WIDTH:
        inv_left = tw - INV_WIDTH
        right_text = "\n".join(_strip_box(r[inv_left:tw]) for r in body_rows)

    return {
        "stats": ("HP:" in left_text) or ("Name:" in left_text),
        "inventory": "You are Carrying:" in right_text,
        "equipment": "You are wearing:" in right_text,
    }


def _parse_dungeon(lines: list[str], term_w: int = 0) -> tuple:
    """
    Extract the dungeon map region (cols STATS_WIDTH–(term_w-INV_WIDTH), rows MSG_HEIGHT+).
    Returns (map_grid, player_pos, monsters, items).
    """
    tw = term_w or TERM_W
    inv_left = tw - INV_WIDTH if tw >= INV_AUTO_WIDTH else tw
    dungeon_rows = lines[MSG_HEIGHT:]
    map_grid = []
    player_pos = None
    monsters = []
    items = []
    rejected_monsters = 0
    rejected_items = 0

    for row_idx, line in enumerate(dungeon_rows):
        row = list(line[STATS_WIDTH:inv_left])
        map_grid.append(row)
        for col_idx, ch in enumerate(row):
            if ch == "@":
                player_pos = (row_idx, col_idx)
            elif ch in _MONSTER_CHARS_DISAMBIGUATED:
                if ch in _ENTITY_EXCLUDE_CHARS or _is_word_like(row, col_idx):
                    rejected_monsters += 1
                    continue
                monsters.append((row_idx, col_idx, ch))
            elif ch in _ITEM_CHARS:
                if (
                    ch in _ENTITY_EXCLUDE_CHARS
                    or _is_text_adjacent(row, col_idx)
                    or ch.isalpha()
                ):
                    rejected_items += 1
                    continue
                items.append((row_idx, col_idx, ch))

    _LAST_PARSE_DEBUG.update(
        {
            "accepted_monsters": len(monsters),
            "accepted_items": len(items),
            "rejected_monsters": rejected_monsters,
            "rejected_items": rejected_items,
        }
    )

    return map_grid, player_pos, monsters, items


def _parse_messages(lines: list[str]) -> str:
    """Return the last non-empty message from the message region.

    The top message box has borders at rows 0 and (MSG_HEIGHT-1); content
    is in rows 1 through MSG_HEIGHT-2. Each content line has a leading 'x'
    border character that we strip.
    """
    content_rows = range(1, MSG_HEIGHT - 1)
    msg_lines = [_strip_box(lines[r]).strip() for r in content_rows]
    non_empty = [l for l in msg_lines if l]
    return non_empty[-1] if non_empty else ""


def _parse_right_panels(lines: list[str], term_w: int = 0) -> tuple[list[tuple[str, str]], list[tuple[str, str]]]:
    """Parse inventory/equipment entries from right sidebar when visible."""
    tw = term_w or TERM_W
    if tw < INV_AUTO_WIDTH:
        return [], []

    inv_left = tw - INV_WIDTH
    body_rows = lines[MSG_HEIGHT:]
    split = len(body_rows) // 2

    inv_rows = body_rows[:split]
    equip_rows = body_rows[split:]

    entry_re = re.compile(r"^\s*([a-z])\s*-\s*(.+?)\s*$")

    def parse_rows(rows: list[str]) -> list[tuple[str, str]]:
        out = []
        for row in rows:
            text = _strip_box(row[inv_left:tw]).strip()
            m = entry_re.match(text)
            if not m:
                continue
            out.append((m.group(1), m.group(2)))
        return out

    return parse_rows(inv_rows), parse_rows(equip_rows)


def read(state=None, dungeon_depth: int = 1) -> GameState:
    """
    Capture the screen and return a fully populated GameState.
    Pass the previous state's dungeon_depth since it isn't shown in the stats panel.
    """
    global _last_term_w
    lines, term_w, term_h = _get_lines()
    _last_term_w = term_w

    stats = _parse_stats(lines, term_w)
    map_grid, player_pos, monsters, items = _parse_dungeon(lines, term_w)
    last_message = _parse_messages(lines)
    inventory, equipment = _parse_right_panels(lines, term_w)

    # Recover player_pos when '@' is hidden (e.g., monster on same tile).
    # Use previous frame's local/world offset to compute current local pos.
    if player_pos is None and stats["world_pos"] and stats["hp"] > 0:
        if state and state.player_pos and state.player_world_pos:
            wx, wy = stats["world_pos"]
            pwx, pwy = state.player_world_pos
            pr, pc = state.player_pos
            lr = pr + (wy - pwy)
            lc = pc + (wx - pwx)
            if 0 <= lr < len(map_grid) and 0 <= lc < len(map_grid[0]):
                player_pos = (lr, lc)

    parsed_depth = dungeon_depth
    if stats["depth_ft"] is not None and stats["depth_ft"] > 0:
        parsed_depth = max(1, stats["depth_ft"] // 50)

    return GameState(
        player_hp=stats["hp"],
        player_max_hp=stats["max_hp"],
        player_ac=stats["ac"],
        player_level=stats["level"],
        damage_dice=stats["damage_dice"],
        to_hit_bonus=stats["to_hit_bonus"],
        to_dam_bonus=stats["to_dam_bonus"],
        dungeon_depth=parsed_depth,
        map=map_grid,
        player_pos=player_pos,
        player_world_pos=stats["world_pos"],
        monsters=monsters,
        items=items,
        inventory=inventory,
        equipment=equipment,
        last_message=last_message,
        raw_lines=lines,
    )


def is_splash_screen(lines: list[str]) -> bool:
    """True when the intro/splash screen is showing (no '@' anywhere)."""
    full = "\n".join(lines)
    return "JMoria" in full and "@" not in full


def is_char_creation(lines: list[str]) -> bool:
    """True when the character creation placeholder is showing."""
    full = "\n".join(lines)
    return "Character Creation" in full


def get_raw_lines() -> list[str]:
    lines, _w, _h = _get_lines()
    return lines


def get_last_parse_debug() -> dict:
    return dict(_LAST_PARSE_DEBUG)
