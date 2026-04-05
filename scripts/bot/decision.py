"""decision.py - Priority decision engine for the JMoria bot (Phase 2)."""

from collections import deque

from . import pathfinding as pf

KEY_TO_DIR = {v: k for k, v in pf.DIR_TO_KEY.items()}


class DecisionEngine:
    def __init__(self):
        self.visited = set()
        self.last_pos = None
        self.stuck_turns = 0
        self.jiggle_idx = 0
        self.last_action = "."
        self.pending_keys = []
        self.spiral_radius = 1
        self.action_history = deque(maxlen=12)
        self.last_world_pos = None
        self.last_map_sig = None
        self.no_progress_turns = 0
        self.escape_idx = 0
        self.world_history = deque(maxlen=16)
        self.cycle_break_cooldown = 0
        self.last_thought = "idle"

    def decide(self, state):
        if not state.player_pos:
            return self._record_decision(".", "no_player_visible")

        if self.cycle_break_cooldown > 0:
            self.cycle_break_cooldown -= 1

        # If we queued a multi-key action (e.g., open + direction), send it first.
        if self.pending_keys:
            action = self.pending_keys.pop(0)
            return self._record_decision(action, "pending_open_direction")

        pos = state.player_pos
        motion_pos = state.player_world_pos if state.player_world_pos is not None else pos
        self.visited.add((state.dungeon_depth, pos[0], pos[1]))

        self._update_progress(state)

        # Detect stuck behavior to break local loops.
        if self.last_pos == motion_pos:
            self.stuck_turns += 1
        else:
            self.stuck_turns = 0
        self.last_pos = motion_pos

        # If we just bumped into a door, issue open-command sequence.
        # 'o' enters open mode and next key is the direction.
        if "bumped into a door" in state.last_message.lower():
            direction = self.last_action if self.last_action in "hjklyubn" else "h"
            self.pending_keys = [direction]
            return self._record_decision("o", f"open_door_then_{direction}")

        # If we bumped into a wall, pivot immediately instead of repeating the same move.
        if "bumped into a wall" in state.last_message.lower():
            pivot = self._pivot_from_wall(state, self.last_action)
            if pivot:
                return self._record_decision(pivot, "wall_bump_pivot")

        # 1) Survival first: rest when low HP and no adjacent threat.
        if state.player_max_hp > 0 and state.hp_pct < 0.30:
            adjacent = None
            if self._monster_signal_reliable(state):
                adjacent = self._adjacent_monster(pos, state.monsters)
            if adjacent is None:
                return self._record_decision("R", "low_hp_rest_no_adjacent_threat")
            flee = pf.key_away_from(pos, adjacent, state.map)
            if flee:
                return self._record_decision(flee, f"low_hp_flee_from_{adjacent}")

        # 2) Immediate combat: bump-attack adjacent monster.
        adjacent = None
        if self._monster_signal_reliable(state):
            adjacent = self._adjacent_monster(pos, state.monsters)
        if adjacent is not None:
            dr = adjacent[0] - pos[0]
            dc = adjacent[1] - pos[1]
            action = pf.DIR_TO_KEY.get((dr, dc), ".")
            return self._record_decision(
                action,
                self._goal_thought(state, "adjacent_attack", adjacent),
            )

        # Break tight patrol cycles (3-4 tile loops) before normal exploration.
        if self._in_patrol_cycle() and self.cycle_break_cooldown == 0:
            self.cycle_break_cooldown = 6
            escape = self._escape_key()
            if escape:
                return self._record_decision(escape, "cycle_detected_escape")

        # 3) Pick up visible nearby items.
        item_goal = pf.find_nearest_target(
            state.map,
            pos,
            lambda ch, p: ch in self._item_chars() and p != pos,
        )
        if item_goal:
            k = self._key_toward(state.map, pos, item_goal)
            if k:
                return self._record_decision(k, self._goal_thought(state, "path_to_item", item_goal))

        # 4) Descend if downstairs visible.
        stair_goal = pf.find_nearest_target(
            state.map,
            pos,
            lambda ch, p: ch == ">",
        )
        if stair_goal:
            k = self._key_toward(state.map, pos, stair_goal)
            if k:
                return self._record_decision(k, self._goal_thought(state, "path_to_stairs", stair_goal))

        # 5) Explore frontier: nearest unvisited walkable tile in viewport.
        frontier = pf.find_nearest_target(
            state.map,
            pos,
            lambda ch, p: pf.is_walkable(ch)
            and (state.dungeon_depth, p[0], p[1]) not in self.visited,
        )
        if frontier:
            k = self._key_toward(state.map, pos, frontier)
            if k:
                return self._record_decision(k, self._goal_thought(state, "path_to_frontier", frontier))

        # 6) Spiral-search fallback to find a farther reachable unvisited tile.
        spiral_key = self._spiral_search_key(state)
        if spiral_key:
            return self._record_decision(spiral_key, "spiral_search")

        # 7) If no visible progress for a while, force an escape pattern.
        if self.no_progress_turns >= 10:
            escape = self._escape_key()
            if escape:
                return self._record_decision(escape, "no_progress_escape")

        # 8) If still stuck, jiggle with directional fallback.
        if self.stuck_turns >= 4:
            key = "hjklyubn"[self.jiggle_idx % 8]
            self.jiggle_idx += 1
            return self._record_decision(key, "stuck_jiggle")

        return self._record_decision(".", "idle_wait")

    def _key_toward(self, grid, start, goal):
        path = pf.path_to(grid, start, goal)
        key = pf.first_step_key(path)
        return self._sanitize_move(key)

    def _record_action(self, action):
        self.last_action = action
        self.action_history.append(action)
        return action

    def _record_decision(self, action, thought):
        self.last_thought = thought
        return self._record_action(action)

    def debug_thought(self):
        return (
            f"{self.last_thought} "
            f"np={self.no_progress_turns} stuck={self.stuck_turns} "
            f"cooldown={self.cycle_break_cooldown}"
        )

    @staticmethod
    def _goal_thought(state, prefix, goal_local):
        # Keep telemetry high-level: target is on-screen, optionally with world estimate.
        thought = f"{prefix}_on_screen"
        if state.player_world_pos is None or state.player_pos is None:
            return thought

        pr, pc = state.player_pos
        gr, gc = goal_local
        wx, wy = state.player_world_pos
        world_goal = (wx + (gc - pc), wy + (gr - pr))
        return f"{prefix}_world~{world_goal}"

    def _sanitize_move(self, key):
        if key is None:
            return None
        if key not in "hjklyubn":
            return key
        if not self.action_history:
            return key

        # Avoid immediate backtracking oscillation unless we are in clear trouble.
        prev = self.action_history[-1]
        if self.no_progress_turns < 8 and self._is_opposite(prev, key):
            for alt in "hjklyubn":
                if alt != key and not self._is_opposite(prev, alt):
                    return alt
        return key

    def _update_progress(self, state):
        world = state.player_world_pos
        if world is not None:
            self.world_history.append(world)
            progressed = (world != self.last_world_pos)
            self.last_world_pos = world
        else:
            map_sig = self._map_signature(state)
            progressed = (map_sig != self.last_map_sig)
            self.last_map_sig = map_sig

        if progressed:
            self.no_progress_turns = 0
        else:
            self.no_progress_turns += 1

    @staticmethod
    def _map_signature(state):
        # Lightweight view signature to detect screen changes when world pos is unavailable.
        if not state.map:
            return ""
        rows = ["".join(r) for r in state.map]
        return "|".join(rows)

    def _escape_key(self):
        pattern = "hjklyubnlykhbnju"
        key = pattern[self.escape_idx % len(pattern)]
        self.escape_idx += 1
        return self._sanitize_move(key)

    def _in_patrol_cycle(self):
        # Detect repeated local loops in world position history.
        if len(self.world_history) >= 8:
            tail = list(self.world_history)[-8:]
            if len(set(tail)) <= 4 and tail[-1] in tail[:-1]:
                return True

        # Detect short repeated action motifs (e.g., l-j-y cycling).
        if len(self.action_history) >= 8:
            tail = list(self.action_history)[-8:]
            a = tail[:4]
            b = tail[4:]
            if a == b and all(k in "hjklyubn" for k in tail):
                return True

        return False

    def _pivot_from(self, last_move):
        """Choose a deterministic alternate move when a wall collision occurs."""
        if last_move not in "hjklyubn":
            return "j"
        pivot_order = {
            "h": "jukbnly",
            "l": "jykbnuh",
            "k": "hlyubnj",
            "j": "hlyubnk",
            "y": "hkjulbn",
            "u": "lkjhnb y".replace(" ", ""),
            "b": "jlhknyu",
            "n": "jlhkuyb",
        }
        for cand in pivot_order.get(last_move, "hjklyubn"):
            if cand != last_move and not self._is_opposite(last_move, cand):
                return self._sanitize_move(cand)
        return self._sanitize_move("j")

    def _pivot_from_wall(self, state, last_move):
        """Pick a wall-escape pivot that is walkable in the current local map."""
        pos = state.player_pos
        if not pos or not state.map:
            return self._pivot_from(last_move)

        if last_move not in "hjklyubn":
            order = "jkhlyubn"
        else:
            # Start with deterministic pivots near the prior heading.
            order = self._pivot_from(last_move) + "hjklyubn"

        rows = len(state.map)
        cols = len(state.map[0]) if rows else 0
        pr, pc = pos

        for cand in order:
            if cand not in KEY_TO_DIR:
                continue
            dr, dc = KEY_TO_DIR[cand]
            nr, nc = pr + dr, pc + dc
            if nr < 0 or nc < 0 or nr >= rows or nc >= cols:
                continue
            if not pf.is_walkable(state.map[nr][nc]):
                continue
            if self.action_history and self._is_opposite(self.action_history[-1], cand):
                continue
            return self._sanitize_move(cand)

        return self._pivot_from(last_move)

    @staticmethod
    def _is_opposite(a, b):
        opposites = {
            "h": "l", "l": "h",
            "j": "k", "k": "j",
            "y": "n", "n": "y",
            "u": "b", "b": "u",
        }
        return opposites.get(a) == b

    def _spiral_search_key(self, state):
        """Find a target by scanning outward in a spiral from current position."""
        if not state.player_pos or not state.map:
            return None

        rows = len(state.map)
        cols = len(state.map[0]) if rows else 0
        if rows == 0 or cols == 0:
            return None

        center = state.player_pos
        max_radius = max(rows, cols)

        # Start search radius from prior attempts to avoid local oscillation.
        start_radius = max(1, self.spiral_radius)

        for radius in range(start_radius, max_radius):
            for r, c in self._spiral_ring(center, radius, rows, cols):
                ch = state.map[r][c]
                if not pf.is_walkable(ch):
                    continue
                if (state.dungeon_depth, r, c) in self.visited:
                    continue
                key = self._key_toward(state.map, center, (r, c))
                if key:
                    # Next time, begin a bit farther out for continued expansion.
                    self.spiral_radius = min(max_radius - 1, radius + 1)
                    return key

        # If no unvisited target exists, allow revisiting walkable spiral points.
        for radius in range(1, max_radius):
            for r, c in self._spiral_ring(center, radius, rows, cols):
                if not pf.is_walkable(state.map[r][c]):
                    continue
                key = self._key_toward(state.map, center, (r, c))
                if key:
                    return key

        return None

    @staticmethod
    def _spiral_ring(center, radius, rows, cols):
        """Yield points on a square spiral ring at distance radius from center."""
        cr, cc = center
        if radius <= 0:
            return

        top = cr - radius
        bottom = cr + radius
        left = cc - radius
        right = cc + radius

        # Top edge (left -> right)
        for c in range(left, right + 1):
            r = top
            if 0 <= r < rows and 0 <= c < cols:
                yield (r, c)

        # Right edge (top+1 -> bottom)
        for r in range(top + 1, bottom + 1):
            c = right
            if 0 <= r < rows and 0 <= c < cols:
                yield (r, c)

        # Bottom edge (right-1 -> left)
        for c in range(right - 1, left - 1, -1):
            r = bottom
            if 0 <= r < rows and 0 <= c < cols:
                yield (r, c)

        # Left edge (bottom-1 -> top+1)
        for r in range(bottom - 1, top, -1):
            c = left
            if 0 <= r < rows and 0 <= c < cols:
                yield (r, c)

    @staticmethod
    def _adjacent_monster(pos, monsters):
        pr, pc = pos
        for mr, mc, _ in monsters:
            if abs(mr - pr) <= 1 and abs(mc - pc) <= 1 and (mr, mc) != (pr, pc):
                return (mr, mc)
        return None

    @staticmethod
    def _monster_signal_reliable(state):
        # Screen parsing can overcount monster-like glyphs in some views.
        # Treat very large counts as noisy and avoid tactical combat decisions from them.
        return len(state.monsters) <= 12

    @staticmethod
    def _item_chars():
        return set(r'|)[](]"=~{}{}&?!-_$~/\\')
