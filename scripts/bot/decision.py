"""decision.py - Priority decision engine for the JMoria bot (Phase 2)."""

from collections import deque
import json
import re

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
        self.pending_wield_slot = None
        self.await_wield_prompt_turns = 0
        self.spiral_radius = 1
        self.action_history = deque(maxlen=12)
        self.last_world_pos = None
        self.last_map_sig = None
        self.no_progress_turns = 0
        self.escape_idx = 0
        self.world_history = deque(maxlen=16)
        self.cycle_break_cooldown = 0
        self.wall_bump_chain = 0
        self.has_wielded_weapon = False
        self.wield_cooldown = 0
        self.equip_attempt_counts = {}
        self.wield_attempt_counts = {}
        self.learned_non_wieldable_names = set()
        self.learned_non_wieldable_tokens = set()
        self.monster_knowledge = {}
        self.scroll_knowledge = {}
        self.pending_scroll_label = None
        self.item_knowledge = {
            "equip_compat": {},
            "light_sources": {},
            "value_estimates": {},
            "gear_strength": {},
            "best_by_slot": {},
        }
        self.door_knowledge = {
            "lock_success": 0,
            "lock_fail": 0,
        }
        self.map_knowledge = {
            "depth_notes": {},
        }
        self.knowledge_dirty = False
        self.last_equip_item_name = None
        self.last_equip_baseline_ac = None
        self.pending_pickup_equip_slot = None
        self.last_inventory_entries = set()
        self.last_depth = 1
        self.last_open_dir = None
        self.follow_open_dir_turns = 0
        self.last_player_hp = None
        self.recent_attacker_name = None
        self.last_thought = "idle"
        self.blocked_dirs_by_world = {}
        self.current_motion_pos = None

    def decide(self, state):
        if not state.player_pos:
            return self._record_decision(".", "no_player_visible")

        if self.cycle_break_cooldown > 0:
            self.cycle_break_cooldown -= 1
        if self.wield_cooldown > 0:
            self.wield_cooldown -= 1
        if self.await_wield_prompt_turns > 0:
            self.await_wield_prompt_turns -= 1
        if self.follow_open_dir_turns > 0:
            self.follow_open_dir_turns -= 1

        pos = state.player_pos
        motion_pos = state.player_world_pos if state.player_world_pos is not None else pos
        self.current_motion_pos = motion_pos
        self.visited.add((state.dungeon_depth, pos[0], pos[1]))

        hp_loss = 0
        if self.last_player_hp is not None:
            hp_loss = max(0, self.last_player_hp - state.player_hp)
        self.last_player_hp = state.player_hp

        if state.dungeon_depth != self.last_depth:
            self.equip_attempt_counts.clear()
            self.last_depth = state.dungeon_depth

        self._update_progress(state)

        # Detect stuck behavior to break local loops.
        if self.last_pos == motion_pos:
            self.stuck_turns += 1
        else:
            self.stuck_turns = 0
        self.last_pos = motion_pos

        # If we just bumped into a door, issue open-command sequence.
        # 'o' enters open mode and next key is the direction.
        msg_lower = state.last_message.lower()
        self._learn_from_wield_feedback(state)
        self._learn_from_monster_feedback(state.last_message, hp_loss)
        self._learn_from_scroll_feedback(state.last_message)
        self._queue_pickup_equip_from_message(state)

        # If we initiated wield, send the slot only when the game prompts for it.
        if self.pending_wield_slot:
            if "wield which item" in msg_lower:
                slot = self.pending_wield_slot
                self.pending_wield_slot = None
                self.await_wield_prompt_turns = 0
                return self._record_decision(slot, f"pending_wield_slot_{slot}")
            if (
                "you are now wielding" in msg_lower
                or "you can't wield" in msg_lower
                or "returns to your pack" in msg_lower
                or self.await_wield_prompt_turns == 0
            ):
                self.pending_wield_slot = None
                self.await_wield_prompt_turns = 0

        # Hard rule: when inventory gains a slot/item, wield that slot immediately.
        if self.pending_pickup_equip_slot and not self.pending_wield_slot:
            pickup_slot = self.pending_pickup_equip_slot
            self.pending_pickup_equip_slot = None
            item_name = self._inventory_name_for_slot(state.inventory, pickup_slot)
            if item_name is None:
                item_name = f"slot_{pickup_slot}"
            self.last_equip_item_name = item_name
            self.last_equip_baseline_ac = state.player_ac
            self.wield_cooldown = 1
            self.wield_attempt_counts[item_name] = self.wield_attempt_counts.get(item_name, 0) + 1
            self.pending_wield_slot = pickup_slot
            self.await_wield_prompt_turns = 3
            return self._record_decision("w", f"inventory_changed_wield_slot_{pickup_slot}")

        # If we queued a multi-key action (e.g., open + direction), send it next.
        if self.pending_keys:
            action = self.pending_keys.pop(0)
            return self._record_decision(action, "pending_open_direction")

        if "you have picked the lock" in msg_lower:
            self.door_knowledge["lock_success"] = int(self.door_knowledge.get("lock_success", 0)) + 1
            self.knowledge_dirty = True
        elif "you failed to pick the lock" in msg_lower:
            self.door_knowledge["lock_fail"] = int(self.door_knowledge.get("lock_fail", 0)) + 1
            self.knowledge_dirty = True

        if "you are now wielding the" in msg_lower:
            self.has_wielded_weapon = True
        if "you were wielding the" in msg_lower:
            self.has_wielded_weapon = False

        if "picked the lock" in msg_lower and isinstance(self.last_open_dir, str) and self.last_open_dir in "hjklyubn":
            self.follow_open_dir_turns = max(self.follow_open_dir_turns, 2)

        if self.follow_open_dir_turns > 0 and self.last_open_dir in "hjklyubn":
            return self._record_decision(self.last_open_dir, f"step_through_open_door_{self.last_open_dir}")

        if "bumped into a door" in msg_lower:
            self.wall_bump_chain = 0
            direction = self.last_action if self.last_action in "hjklyubn" else "h"
            self.last_open_dir = direction
            self.follow_open_dir_turns = 0
            self.pending_keys = [direction]
            return self._record_decision("o", f"open_door_then_{direction}")

        # If we bumped into a wall, pivot immediately instead of repeating the same move.
        if "bumped into a wall" in msg_lower:
            if self.last_action in "hjklyubn":
                self._mark_blocked_dir(self.last_action)
            self.wall_bump_chain += 1
            if self.wall_bump_chain >= 6:
                self.wall_bump_chain = 0
                self.cycle_break_cooldown = max(self.cycle_break_cooldown, 6)
                escape = self._escape_key()
                if escape:
                    return self._record_decision(escape, "wall_bump_chain_escape")
            pivot = self._pivot_from_wall(state, self.last_action)
            if pivot:
                return self._record_decision(pivot, "wall_bump_pivot")
        else:
            self.wall_bump_chain = 0

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

        # Precompute adjacent threat for equip/combat ordering.
        adjacent = None
        if self._monster_signal_reliable(state):
            adjacent = self._adjacent_monster(pos, state.monsters)

        # 2) No background wield-cycling: wield is pickup-driven to preserve movement.

        # 3) Immediate combat: bump-attack adjacent monster.
        if adjacent is not None:
            threat_name = self.recent_attacker_name
            danger = self._monster_danger_score(threat_name) if threat_name else 0.0
            confidence = self._monster_confidence(threat_name) if threat_name else 0.0
            if self._should_flee_known_threat(state, danger, confidence):
                flee = pf.key_away_from(pos, adjacent, state.map)
                if flee:
                    return self._record_decision(
                        flee,
                        f"flee_known_threat_{threat_name}_d{danger:.2f}_c{confidence:.2f}",
                    )
            dr = adjacent[0] - pos[0]
            dc = adjacent[1] - pos[1]
            action = pf.DIR_TO_KEY.get((dr, dc), ".")
            return self._record_decision(
                action,
                f"{self._goal_thought(state, 'adjacent_attack', adjacent)}_d{danger:.2f}_c{confidence:.2f}",
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

        # 4) Doors are high-value exploration targets in larger rooms.
        door_dir = self._adjacent_door_direction(state.map, pos)
        if door_dir:
            self.pending_keys = [door_dir]
            return self._record_decision("o", f"open_adjacent_door_{door_dir}")

        door_goal = self._nearest_door_approach(state.map, pos)
        if door_goal:
            k = self._key_toward(state.map, pos, door_goal)
            if k:
                return self._record_decision(k, self._goal_thought(state, "path_to_door", door_goal))

        # 5) Descend if downstairs visible.
        stair_goal = pf.find_nearest_target(
            state.map,
            pos,
            lambda ch, p: ch == ">",
        )
        if stair_goal:
            k = self._key_toward(state.map, pos, stair_goal)
            if k:
                return self._record_decision(k, self._goal_thought(state, "path_to_stairs", stair_goal))

        # 6) Explore frontier: nearest unvisited walkable tile in viewport.
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

        # 7) Spiral-search fallback to find a farther reachable unvisited tile.
        spiral_key = self._spiral_search_key(state)
        if spiral_key:
            return self._record_decision(spiral_key, "spiral_search")

        # 8) If no visible progress for a while, force an escape pattern.
        if self.no_progress_turns >= 10:
            escape = self._escape_key()
            if escape:
                return self._record_decision(escape, "no_progress_escape")

        # 9) If still stuck, jiggle with directional fallback.
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
            f"cooldown={self.cycle_break_cooldown} wb={self.wall_bump_chain}"
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

        if self._is_blocked_dir(key):
            for alt in "hjklyubn":
                if alt != key and not self._is_blocked_dir(alt):
                    key = alt
                    break

        if not self.action_history:
            return key

        # Avoid immediate backtracking oscillation unless we are in clear trouble.
        prev = self.action_history[-1]
        if self.no_progress_turns < 8 and self._is_opposite(prev, key):
            for alt in "hjklyubn":
                if alt != key and not self._is_opposite(prev, alt) and not self._is_blocked_dir(alt):
                    return alt
        return key

    def _mark_blocked_dir(self, key):
        if self.current_motion_pos is None or key not in "hjklyubn":
            return
        entry = self.blocked_dirs_by_world.setdefault(self.current_motion_pos, set())
        entry.add(key)

    def _is_blocked_dir(self, key):
        if self.current_motion_pos is None or key not in "hjklyubn":
            return False
        entry = self.blocked_dirs_by_world.get(self.current_motion_pos)
        return bool(entry and key in entry)

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
    def _adjacent_door_direction(grid, pos):
        pr, pc = pos
        for key in "hjklyubn":
            dr, dc = KEY_TO_DIR[key]
            nr, nc = pr + dr, pc + dc
            if nr < 0 or nc < 0 or nr >= len(grid) or nc >= len(grid[0]):
                continue
            if grid[nr][nc] == "+":
                return key
        return None

    def _nearest_door_approach(self, grid, pos):
        return pf.find_nearest_target(
            grid,
            pos,
            lambda ch, p: pf.is_walkable(ch)
            and p != pos
            and self._adjacent_door_direction(grid, p) is not None,
        )

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

    def _next_equip_candidate(self, inventory, equipment):
        if not inventory:
            return None

        score_table = {
            "torch": 12,
            "lantern": 12,
            "great sword": 11,
            "long sword": 10,
            "broad sword": 10,
            "battle axe": 9,
            "leather armor": 9,
            "chain mail": 9,
            "plate armor": 10,
            "shield": 8,
            "helm": 7,
            "cap": 6,
            "cloak": 6,
            "boots": 6,
            "gauntlets": 6,
            "gloves": 6,
            "mace": 8,
            "war hammer": 8,
            "morning star": 8,
            "spear": 7,
            "dagger": 6,
            "whip": 5,
            "club": 4,
            "pickaxe": 3,
            "shovel": 2,
        }

        best = None
        best_rank = None
        equipped_names = {name.lower() for _slot, name in (equipment or [])}
        equipped_by_slot = {}
        for _slot, name in (equipment or []):
            kind = self._gear_slot_kind(name)
            if not kind:
                continue
            cur = equipped_by_slot.get(kind)
            score = self._gear_score(name)
            if cur is None or score > cur[1]:
                equipped_by_slot[kind] = (name, score)

        for slot, name in inventory:
            n = name.lower()
            if n in equipped_names:
                continue
            if n in self.learned_non_wieldable_names:
                continue
            if any(tok in n for tok in self.learned_non_wieldable_tokens):
                continue
            if self.wield_attempt_counts.get(name, 0) >= 6:
                continue

            kind = self._gear_slot_kind(name)

            base = 0
            for key, val in score_table.items():
                if key in n:
                    base = max(base, val)
            if any(k in n for k in ("bow", "sling", "crossbow")):
                base = max(base, 6)

            total_score = self._gear_score(name)
            equipped_score = equipped_by_slot.get(kind, (None, 0))[1] if kind else 0
            improvement = total_score - equipped_score if kind else 0.0
            attempts = self.wield_attempt_counts.get(name, 0)
            known = self.item_knowledge.get("gear_strength", {}).get(n, {})
            successes = int(known.get("successes", 0)) if isinstance(known, dict) else 0
            is_untested = attempts == 0 and successes == 0
            known_best = 1 if (kind and self._is_known_best_for_slot(kind, name)) else 0
            is_gear = 1 if kind else 0

            # Rank tuple favors: untested items first, then known best gear and upgrades.
            rank = (
                1 if is_untested else 0,
                known_best,
                is_gear,
                1 if improvement > 0.25 else 0,
                float(improvement),
                float(total_score),
                float(base),
                -float(attempts),
            )

            if best_rank is None or rank > best_rank:
                best = (slot, name, total_score)
                best_rank = rank

        return best

    def _gear_slot_kind(self, item_name):
        n = item_name.lower()
        if any(k in n for k in ("torch", "lantern")):
            return "light"
        if any(k in n for k in ("shield",)):
            return "shield"
        if any(k in n for k in ("boots",)):
            return "boots"
        if any(k in n for k in ("helm", "cap")):
            return "head"
        if any(k in n for k in ("cloak",)):
            return "cloak"
        if any(k in n for k in ("gauntlets", "gloves")):
            return "hands"
        if any(k in n for k in ("armor", "mail", "robe")):
            return "body"
        if any(
            k in n
            for k in (
                "sword",
                "axe",
                "mace",
                "hammer",
                "morning star",
                "spear",
                "dagger",
                "whip",
                "club",
                "pickaxe",
                "shovel",
                "bow",
                "sling",
                "crossbow",
            )
        ):
            return "weapon"
        return None

    def _gear_score(self, item_name):
        n = item_name.lower()
        score = 0.0
        score_table = {
            "torch": 12,
            "lantern": 12,
            "great sword": 11,
            "long sword": 10,
            "broad sword": 10,
            "battle axe": 9,
            "leather armor": 9,
            "chain mail": 9,
            "plate armor": 10,
            "shield": 8,
            "helm": 7,
            "cap": 6,
            "cloak": 6,
            "boots": 6,
            "gauntlets": 6,
            "gloves": 6,
            "mace": 8,
            "war hammer": 8,
            "morning star": 8,
            "spear": 7,
            "dagger": 6,
            "whip": 5,
            "club": 4,
            "pickaxe": 3,
            "shovel": 2,
        }
        for key, val in score_table.items():
            if key in n:
                score = max(score, float(val))
        if any(k in n for k in ("bow", "sling", "crossbow")):
            score = max(score, 6.0)

        gear_strength = self.item_knowledge.get("gear_strength", {})
        learned = gear_strength.get(n, {}) if isinstance(gear_strength, dict) else {}
        if isinstance(learned, dict):
            score += float(learned.get("observed_ac_best", 0)) * 3.0
            score += float(learned.get("observed_ac_avg", 0)) * 1.2
            score += min(0.8, float(learned.get("successes", 0)) * 0.1)
        return score

    @staticmethod
    def _inventory_name_for_slot(inventory, slot):
        for s, name in inventory or []:
            if s == slot:
                return name
        return None

    def _is_known_best_for_slot(self, kind, item_name):
        best_by_slot = self.item_knowledge.get("best_by_slot", {})
        if not isinstance(best_by_slot, dict):
            return False
        entry = best_by_slot.get(kind)
        if not isinstance(entry, dict):
            return False
        return str(entry.get("name", "")).lower() == item_name.lower()

    def _learn_from_wield_feedback(self, state):
        message = state.last_message
        if not message:
            return

        lower = message.lower()

        # Explicit failure: "You can't wield a Scroll ...!"
        m = re.search(r"you can't wield a\s+(.+?)!", lower)
        if m:
            item = m.group(1).strip()
            if item:
                self._learn_non_wieldable(item)
            return

        # Wield attempt bounced back to inventory.
        if "returns to your pack" in lower and self.last_equip_item_name:
            item = self.last_equip_item_name.lower()
            self._learn_non_wieldable(item)
            self.wield_attempt_counts[item] = 99
            self.last_equip_item_name = None
            self.last_equip_baseline_ac = None
            return

        # Successful equip: learn AC impact and track best-known item per slot.
        m = re.search(r"you are now wielding the\s+(.+?)\.?$", message, flags=re.IGNORECASE)
        if m:
            item = m.group(1).strip()
            if item:
                self._learn_successful_equip(item, state.player_ac)

    def _learn_successful_equip(self, item_name, current_ac):
        n = item_name.lower()
        gear_strength = self.item_knowledge.setdefault("gear_strength", {})
        if not isinstance(gear_strength, dict):
            gear_strength = {}
            self.item_knowledge["gear_strength"] = gear_strength

        entry = gear_strength.setdefault(
            n,
            {
                "successes": 0,
                "observed_ac_total": 0,
                "observed_ac_count": 0,
                "observed_ac_avg": 0.0,
                "observed_ac_best": 0,
            },
        )

        entry["successes"] = int(entry.get("successes", 0)) + 1

        if self.last_equip_baseline_ac is not None:
            delta = int(current_ac) - int(self.last_equip_baseline_ac)
            entry["observed_ac_total"] = int(entry.get("observed_ac_total", 0)) + delta
            entry["observed_ac_count"] = int(entry.get("observed_ac_count", 0)) + 1
            count = max(1, int(entry.get("observed_ac_count", 0)))
            total = int(entry.get("observed_ac_total", 0))
            entry["observed_ac_avg"] = total / count
            entry["observed_ac_best"] = max(int(entry.get("observed_ac_best", 0)), delta)

        kind = self._gear_slot_kind(item_name)
        if kind:
            best_by_slot = self.item_knowledge.setdefault("best_by_slot", {})
            current_best = best_by_slot.get(kind)
            new_score = self._gear_score(item_name)
            if not isinstance(current_best, dict) or float(current_best.get("score", -1e9)) < new_score:
                best_by_slot[kind] = {"name": item_name, "score": new_score}

        self.knowledge_dirty = True
        self.last_equip_item_name = None
        self.last_equip_baseline_ac = None

    def _queue_pickup_equip_from_message(self, state):
        msg = (state.last_message or "").strip()
        if not msg:
            return
        m = re.search(r"you have an?\s+(.+?)\.?$", msg, flags=re.IGNORECASE)
        if not m:
            return
        picked = m.group(1).strip().lower()
        if not picked:
            return

        # Prefer exact match to the picked-up item name.
        for slot, name in state.inventory or []:
            n = name.lower()
            if n == picked:
                self.pending_pickup_equip_slot = slot
                return

        # Fallback: parser may truncate/wrap names; choose first inventory slot.
        if state.inventory:
            self.pending_pickup_equip_slot = state.inventory[0][0]

    def _queue_new_inventory_equips(self, state):
        current_entries = {f"{slot}:{name}" for slot, name in (state.inventory or [])}
        new_entries = current_entries - self.last_inventory_entries
        self.last_inventory_entries = current_entries
        if self.pending_pickup_equip_slot or not new_entries:
            return

        for entry in sorted(new_entries):
            slot, _, name = entry.partition(":")
            if not slot or not name:
                continue
            self.pending_pickup_equip_slot = slot
            return

    def _learn_non_wieldable(self, item_name):
        before_names = len(self.learned_non_wieldable_names)
        before_tokens = len(self.learned_non_wieldable_tokens)
        self.learned_non_wieldable_names.add(item_name.lower())
        self._learn_item_token(item_name)
        if (
            len(self.learned_non_wieldable_names) != before_names
            or len(self.learned_non_wieldable_tokens) != before_tokens
        ):
            self.knowledge_dirty = True

    def _learn_from_monster_feedback(self, message, hp_loss=0):
        if not message:
            return

        msg = message.strip()

        # You hit the Giant Ant.
        m = re.search(r"you hit the\s+(.+?)\.?$", msg, flags=re.IGNORECASE)
        if m:
            self._monster_note(m.group(1), "hits", 1)
            return

        # You miss the Giant Ant.
        m = re.search(r"you miss the\s+(.+?)\.?$", msg, flags=re.IGNORECASE)
        if m:
            self._monster_note(m.group(1), "misses", 1)
            return

        # You have slain the Giant Ant.
        m = re.search(r"you have slain the\s+(.+?)\.?$", msg, flags=re.IGNORECASE)
        if m:
            self._monster_note(m.group(1), "kills", 1)
            return

        # The Giant Ant bites/touches/claws/breathes ...
        m = re.search(
            r"the\s+(.+?)\s+(bites|claws|touches|hits|breathes|stings|kicks|gazes|spits)\b",
            msg,
            flags=re.IGNORECASE,
        )
        if m:
            monster = m.group(1)
            attack = m.group(2).lower()
            self.recent_attacker_name = monster.strip().lower()
            self._monster_note(monster, "hits_taken", 1)
            self._monster_attack_note(monster, attack)
            if hp_loss > 0:
                self._monster_damage_note(monster, hp_loss)

    def _monster_note(self, monster_name, key, amount):
        name = monster_name.strip().lower()
        if not name:
            return
        entry = self.monster_knowledge.setdefault(
            name,
            {
                "hits": 0,
                "misses": 0,
                "hits_taken": 0,
                "kills": 0,
                "attacks": {},
                "damage_taken_total": 0,
                "damage_instances": 0,
                "max_observed_hit": 0,
            },
        )
        before = entry.get(key, 0)
        entry[key] = before + amount
        if entry[key] != before:
            self.knowledge_dirty = True

    def _monster_attack_note(self, monster_name, attack_type):
        name = monster_name.strip().lower()
        if not name:
            return
        entry = self.monster_knowledge.setdefault(
            name,
            {
                "hits": 0,
                "misses": 0,
                "hits_taken": 0,
                "kills": 0,
                "attacks": {},
                "damage_taken_total": 0,
                "damage_instances": 0,
                "max_observed_hit": 0,
            },
        )
        attacks = entry.setdefault("attacks", {})
        before = attacks.get(attack_type, 0)
        attacks[attack_type] = before + 1
        if attacks[attack_type] != before:
            self.knowledge_dirty = True

    def _monster_damage_note(self, monster_name, hp_loss):
        name = monster_name.strip().lower()
        if not name or hp_loss <= 0:
            return
        entry = self.monster_knowledge.setdefault(
            name,
            {
                "hits": 0,
                "misses": 0,
                "hits_taken": 0,
                "kills": 0,
                "attacks": {},
                "damage_taken_total": 0,
                "damage_instances": 0,
                "max_observed_hit": 0,
            },
        )
        entry["damage_taken_total"] = int(entry.get("damage_taken_total", 0)) + int(hp_loss)
        entry["damage_instances"] = int(entry.get("damage_instances", 0)) + 1
        entry["max_observed_hit"] = max(int(entry.get("max_observed_hit", 0)), int(hp_loss))
        self.knowledge_dirty = True

    def _monster_danger_score(self, monster_name):
        if not monster_name:
            return 0.0
        entry = self.monster_knowledge.get(monster_name.lower(), {})
        hits_taken = float(entry.get("hits_taken", 0))
        instances = float(entry.get("damage_instances", 0))
        dmg_total = float(entry.get("damage_taken_total", 0))
        max_hit = float(entry.get("max_observed_hit", 0))
        avg_dmg = (dmg_total / instances) if instances > 0 else 0.0
        hit_pressure = (hits_taken / max(1.0, hits_taken + float(entry.get("misses", 0))))
        attacks = entry.get("attacks", {}) or {}
        breath_bonus = 1.0 if attacks.get("breathes", 0) else 0.0
        return (avg_dmg * 1.4) + (max_hit * 0.6) + (hit_pressure * 2.0) + breath_bonus

    def _monster_confidence(self, monster_name):
        if not monster_name:
            return 0.0
        entry = self.monster_knowledge.get(monster_name.lower(), {})
        samples = float(entry.get("damage_instances", 0)) + float(entry.get("hits_taken", 0))
        # Saturates toward 1.0 with more observations.
        return min(1.0, samples / 6.0)

    def _should_flee_known_threat(self, state, danger, confidence):
        if state.hp_pct > 0.65 or not self.recent_attacker_name:
            return False

        # Conservative with low confidence, more decisive as confidence rises.
        if state.hp_pct < 0.35 and danger >= 0.8:
            return True
        if confidence >= 0.5 and state.hp_pct < 0.55 and danger >= 1.6:
            return True
        if confidence >= 0.8 and state.hp_pct < 0.65 and danger >= 2.4:
            return True
        return False

    def _learn_from_scroll_feedback(self, message):
        if not message:
            return

        msg = message.strip()

        # Reading event with randomized label, e.g. "You read the Scroll labeled foo."
        m = re.search(r"you read the\s+scroll labeled\s+(.+?)\.?$", msg, flags=re.IGNORECASE)
        if m:
            label = m.group(1).strip().lower()
            if label:
                self.pending_scroll_label = label
            return

        if not self.pending_scroll_label:
            return

        lower = msg.lower()
        # Skip transitional/system lines.
        if any(
            k in lower
            for k in (
                "choose an item",
                "you read the",
                "you are now",
                "you have",
                "you miss",
                "you hit",
            )
        ):
            return

        # Capture first meaningful post-read effect text as learned effect note.
        label_entry = self.scroll_knowledge.setdefault(self.pending_scroll_label, {"effects": [], "count": 0})
        effects = label_entry.setdefault("effects", [])
        if msg not in effects:
            effects.append(msg)
            self.knowledge_dirty = True
        label_entry["count"] = int(label_entry.get("count", 0)) + 1
        self.knowledge_dirty = True
        self.pending_scroll_label = None

    def _learn_item_token(self, item_name):
        # Learn noun-like tokens (e.g., scroll, potion) to generalize future filtering.
        stop = {"set", "pair", "of", "the", "a", "an", "labeled"}
        tokens = [t for t in re.findall(r"[a-z]+", item_name.lower()) if t not in stop]
        for t in tokens:
            if len(t) >= 4:
                self.learned_non_wieldable_tokens.add(t)

    def load_knowledge(self, file_path):
        try:
            with open(file_path, "r", encoding="utf-8") as f:
                data = json.load(f)
            names = data.get("non_wieldable_names", [])
            tokens = data.get("non_wieldable_tokens", [])
            monsters = data.get("monster_knowledge", {})
            scrolls = data.get("scroll_knowledge", {})
            item_k = data.get("item_knowledge", {})
            door_k = data.get("door_knowledge", {})
            map_k = data.get("map_knowledge", {})
            self.learned_non_wieldable_names.update(n.lower() for n in names if isinstance(n, str))
            self.learned_non_wieldable_tokens.update(t.lower() for t in tokens if isinstance(t, str))
            if isinstance(monsters, dict):
                self.monster_knowledge = monsters
            if isinstance(scrolls, dict):
                self.scroll_knowledge = scrolls
            if isinstance(item_k, dict):
                self.item_knowledge = item_k
                self.item_knowledge.setdefault("gear_strength", {})
                self.item_knowledge.setdefault("best_by_slot", {})
            if isinstance(door_k, dict):
                self.door_knowledge = door_k
            if isinstance(map_k, dict):
                self.map_knowledge = map_k
            self.knowledge_dirty = False
            return True
        except FileNotFoundError:
            return False
        except Exception:
            return False

    def save_knowledge(self, file_path):
        data = {
            "schema_version": 3,
            "non_wieldable_names": sorted(self.learned_non_wieldable_names),
            "non_wieldable_tokens": sorted(self.learned_non_wieldable_tokens),
            "monster_knowledge": self.monster_knowledge,
            "scroll_knowledge": self.scroll_knowledge,
            "item_knowledge": self.item_knowledge,
            "door_knowledge": self.door_knowledge,
            "map_knowledge": self.map_knowledge,
        }
        with open(file_path, "w", encoding="utf-8") as f:
            json.dump(data, f, indent=2, sort_keys=True)
            f.write("\n")
        self.knowledge_dirty = False
