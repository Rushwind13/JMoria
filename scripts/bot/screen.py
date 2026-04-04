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

from .state import GameState

SESSION = "crawler"

# Terminal dimensions we launch with (must match crawler.py)
TERM_W = 125
TERM_H = 40

# Layout constants mirroring ASCIILayout::CreateForSize
MSG_HEIGHT = 5
STATS_WIDTH = 25
INV_WIDTH = 25
INV_AUTO_WIDTH = 100

# Monster chars: MonIDs from Monster.cpp
# "abcddefghhikllmnoprsuwxyzABCDFFFGGHIJKLOPRSTUVWWXY&.,$t"
# We treat any letter (a-z, A-Z) in the dungeon region as a monster.
# Items are non-letter, non-space, non-'@', non-'#', non-'.', non-'+',
# non-"'", non-'<', non-'>', non-':' symbols.
_ITEM_CHARS = set(r'|)[](]\"=~{}{}&?!-_$~/\\')


def _get_lines() -> list[str]:
    """Capture the tmux pane and return lines padded to TERM_W."""
    result = subprocess.run(
        ["tmux", "capture-pane", "-t", SESSION, "-p"],
        check=True,
        capture_output=True,
        text=True,
    )
    lines = result.stdout.splitlines()
    # Pad to TERM_H rows, each padded to TERM_W cols
    while len(lines) < TERM_H:
        lines.append("")
    lines = lines[:TERM_H]
    lines = [line.ljust(TERM_W) for line in lines]
    return lines


def _strip_box(text: str) -> str:
    """Strip ncurses box-drawing border chars from a string."""
    return text.lstrip("lmxqtuvwj+|").rstrip("lmxqktuvwj+|")


def _parse_stats(lines: list[str]) -> dict:
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

    return {
        "hp": hp,
        "max_hp": max_hp,
        "ac": ac,
        "level": level,
        "depth_ft": depth_ft,
        "world_pos": world_pos,
    }


def _parse_dungeon(lines: list[str]) -> tuple:
    """
    Extract the dungeon map region (cols STATS_WIDTH–(TERM_W-INV_WIDTH), rows MSG_HEIGHT+).
    Returns (map_grid, player_pos, monsters, items).
    """
    inv_left = TERM_W - INV_WIDTH if TERM_W >= INV_AUTO_WIDTH else TERM_W
    dungeon_rows = lines[MSG_HEIGHT:]
    map_grid = []
    player_pos = None
    monsters = []
    items = []

    for row_idx, line in enumerate(dungeon_rows):
        row = list(line[STATS_WIDTH:inv_left])
        map_grid.append(row)
        for col_idx, ch in enumerate(row):
            if ch == "@":
                player_pos = (row_idx, col_idx)
            elif ch.isalpha():
                monsters.append((row_idx, col_idx, ch))
            elif ch in _ITEM_CHARS:
                items.append((row_idx, col_idx, ch))

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


def read(state=None, dungeon_depth: int = 1) -> GameState:
    """
    Capture the screen and return a fully populated GameState.
    Pass the previous state's dungeon_depth since it isn't shown in the stats panel.
    """
    lines = _get_lines()

    stats = _parse_stats(lines)
    map_grid, player_pos, monsters, items = _parse_dungeon(lines)
    last_message = _parse_messages(lines)

    parsed_depth = dungeon_depth
    if stats["depth_ft"] is not None and stats["depth_ft"] > 0:
        parsed_depth = max(1, stats["depth_ft"] // 50)

    return GameState(
        player_hp=stats["hp"],
        player_max_hp=stats["max_hp"],
        player_ac=stats["ac"],
        player_level=stats["level"],
        dungeon_depth=parsed_depth,
        map=map_grid,
        player_pos=player_pos,
        player_world_pos=stats["world_pos"],
        monsters=monsters,
        items=items,
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
    return _get_lines()
