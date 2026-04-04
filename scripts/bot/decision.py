"""decision.py - Priority decision engine for the JMoria bot (Phase 2)."""

from . import pathfinding as pf


class DecisionEngine:
    def __init__(self):
        self.visited = set()
        self.last_pos = None
        self.stuck_turns = 0
        self.jiggle_idx = 0
        self.last_action = "."
        self.pending_keys = []

    def decide(self, state):
        if not state.player_pos:
            return "."

        # If we queued a multi-key action (e.g., open + direction), send it first.
        if self.pending_keys:
            action = self.pending_keys.pop(0)
            self.last_action = action
            return action

        pos = state.player_pos
        self.visited.add((state.dungeon_depth, pos[0], pos[1]))

        # Detect stuck behavior to break local loops.
        if self.last_pos == pos:
            self.stuck_turns += 1
        else:
            self.stuck_turns = 0
        self.last_pos = pos

        # If we just bumped into a door, issue open-command sequence.
        # 'o' enters open mode and next key is the direction.
        if "bumped into a door" in state.last_message.lower():
            direction = self.last_action if self.last_action in "hjklyubn" else "h"
            self.pending_keys = [direction]
            self.last_action = "o"
            return "o"

        # 1) Survival first: rest when low HP and no adjacent threat.
        if state.player_max_hp > 0 and state.hp_pct < 0.30:
            adjacent = self._adjacent_monster(pos, state.monsters)
            if adjacent is None:
                self.last_action = "R"
                return "R"
            flee = pf.key_away_from(pos, adjacent, state.map)
            if flee:
                self.last_action = flee
                return flee

        # 2) Immediate combat: bump-attack adjacent monster.
        adjacent = self._adjacent_monster(pos, state.monsters)
        if adjacent is not None:
            dr = adjacent[0] - pos[0]
            dc = adjacent[1] - pos[1]
            action = pf.DIR_TO_KEY.get((dr, dc), ".")
            self.last_action = action
            return action

        # 3) Pick up visible nearby items.
        item_goal = pf.find_nearest_target(
            state.map,
            pos,
            lambda ch, p: ch in self._item_chars() and p != pos,
        )
        if item_goal:
            k = self._key_toward(state.map, pos, item_goal)
            if k:
                self.last_action = k
                return k

        # 4) Descend if downstairs visible.
        stair_goal = pf.find_nearest_target(
            state.map,
            pos,
            lambda ch, p: ch == ">",
        )
        if stair_goal:
            k = self._key_toward(state.map, pos, stair_goal)
            if k:
                self.last_action = k
                return k

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
                self.last_action = k
                return k

        # 6) If stuck too long, jiggle with directional fallback.
        if self.stuck_turns >= 4:
            key = "hjklyubn"[self.jiggle_idx % 8]
            self.jiggle_idx += 1
            self.last_action = key
            return key

        self.last_action = "."
        return "."

    def _key_toward(self, grid, start, goal):
        path = pf.path_to(grid, start, goal)
        return pf.first_step_key(path)

    @staticmethod
    def _adjacent_monster(pos, monsters):
        pr, pc = pos
        for mr, mc, _ in monsters:
            if abs(mr - pr) <= 1 and abs(mc - pc) <= 1 and (mr, mc) != (pr, pc):
                return (mr, mc)
        return None

    @staticmethod
    def _item_chars():
        return set(r'|)[](]"=~{}{}&?!-_$~/\\')
