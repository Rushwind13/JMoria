"""
state.py — Data classes representing parsed game state.
"""

from dataclasses import dataclass, field
from typing import Optional


@dataclass
class GameState:
    # Player stats (parsed from left stats sidebar)
    player_hp: int = 0
    player_max_hp: int = 0
    player_ac: int = 0
    player_level: int = 0
    damage_dice: str = ""       # e.g. "1d2" from stats panel
    to_hit_bonus: int = 0       # +to Hit modifier
    to_dam_bonus: int = 0       # +to Dam modifier

    # Dungeon depth (tracked from message text, not in stats panel)
    dungeon_depth: int = 1      # level 1 = 50 ft

    # Visible map (2D char grid from dungeon region)
    # Indexed as map[row][col]. '@' = player, letters = monsters, symbols = items
    map: list = field(default_factory=list)

    # Player position in map grid coordinates (row, col)
    player_pos: Optional[tuple] = None

    # Player absolute world position in dungeon coordinates (x, y), if available
    player_world_pos: Optional[tuple] = None

    # Visible monsters: list of (row, col, char)
    monsters: list = field(default_factory=list)

    # Visible items: list of (row, col, char)
    items: list = field(default_factory=list)

    # Parsed right-panel lists: (slot_letter, display_name)
    inventory: list = field(default_factory=list)
    equipment: list = field(default_factory=list)

    # Last message line from the message region (top of screen)
    last_message: str = ""

    # Raw lines of the last capture (for debugging)
    raw_lines: list = field(default_factory=list)

    @property
    def hp_pct(self) -> float:
        if self.player_max_hp == 0:
            return 1.0
        return self.player_hp / self.player_max_hp

    @property
    def is_dead(self) -> bool:
        return self.player_hp <= 0

    @property
    def is_in_game(self) -> bool:
        """True when the dungeon map region contains the player '@'."""
        return self.player_pos is not None
