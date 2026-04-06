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
        self.action_history = deque(maxlen=12)
        self.last_world_pos = None
        self.last_map_sig = None
        self.no_progress_turns = 0
        self.world_history = deque(maxlen=16)
        self.has_wielded_weapon = False
        self.wield_cooldown = 0
        self.equip_attempt_counts = {}
        self.wield_attempt_counts = {}
        self.learned_non_wieldable_categories = set()
        self.monster_knowledge = {}
        self.scroll_knowledge = {}
        self.pending_scroll_label = None
        self.consumable_knowledge = {}
        self.flavor_map = {}  # per-run: flavor name -> true identity or observed effect
        self.pending_use_cmd = None  # 'q', 'r', or 'z'
        self.pending_use_slot = None
        self.pending_use_item_name = None
        self.await_use_prompt_turns = 0
        self.pending_consumable_flavor = None  # flavor awaiting effect observation
        self.pre_use_hp = None  # HP snapshot taken before a consumable use
        self.use_cooldown = 0
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
        self.last_equip_baseline_damage = ""
        self.pending_pickup_equip_slot = None
        self.pending_reequip_kind = None  # slot kind to swap back to best after testing
        self.last_inventory_entries = set()
        self.last_depth = 1
        self.last_open_dir = None
        self.last_player_hp = None
        self.recent_attacker_name = None
        self.last_thought = "idle"
        self.current_wielded_weapon = None
        self.weapon_combat_turn = 0
        self.turns_since_combat_feedback = 99  # high = no recent feedback
        self.phantom_positions = set()  # world positions of suspected phantoms
        self.current_motion_pos = None
        self.current_map = None
        self.current_pos = None
        # Wall-follow: head east until wall, then clockwise wall hug.
        self.explore_phase = "head_east"       # "head_east" or "wall_follow"
        self.wall_follow_heading = None          # current heading key during wall follow
        self.wall_follow_start_wpos = None       # world pos where wall follow began
        self.wall_follow_started = False         # True after first step away from start
        # Retained for door/lock feedback and helper method compatibility.
        self.failed_door_dirs_by_world = {}
        self.blocked_dirs_by_world = {}
        self.wall_bump_chain = 0
        self.follow_open_dir_turns = 0
        self.avoid_key = None
        self.avoid_key_turns = 0
        self.cycle_escalation = 0
        self.mode = "seek"

    def decide(self, state):
        self.mode = "seek"
        if not state.player_pos:
            self.mode = "idle"
            return self._record_decision(".", "no_player_visible")

        if self.wield_cooldown > 0:
            self.wield_cooldown -= 1
        if self.use_cooldown > 0:
            self.use_cooldown -= 1
        if self.await_wield_prompt_turns > 0:
            self.await_wield_prompt_turns -= 1

        pos = state.player_pos
        motion_pos = state.player_world_pos if state.player_world_pos is not None else pos
        self.current_motion_pos = motion_pos
        self.current_map = state.map
        self.current_pos = pos
        self.visited.add((state.dungeon_depth, pos[0], pos[1]))

        hp_loss = 0
        if self.last_player_hp is not None:
            hp_loss = max(0, self.last_player_hp - state.player_hp)
        self.last_player_hp = state.player_hp

        if state.dungeon_depth != self.last_depth:
            self.equip_attempt_counts.clear()
            self.phantom_positions.clear()
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
        self._learn_from_consumable_feedback(state)
        self._queue_pickup_equip_from_message(state)
        if "you have a " in msg_lower or "you have an " in msg_lower:
            self._queue_new_inventory_equips(state)

        adjacent = None
        if self._monster_signal_reliable(state):
            adjacent = self._adjacent_monster_filtered(state, pos, state.monsters)

        # If we initiated wield, send the slot only when the game prompts for it.
        if self.pending_wield_slot:
            self.mode = "equip"
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

        # If we initiated a consumable use (q/r), send the slot when prompted.
        if self.pending_use_slot:
            self.mode = "use"
            prompt_match = (
                ("quaff which item" in msg_lower and self.pending_use_cmd == "q")
                or ("read which item" in msg_lower and self.pending_use_cmd == "r")
            )
            if prompt_match:
                slot = self.pending_use_slot
                cmd = self.pending_use_cmd
                self.pending_use_slot = None
                self.pending_use_cmd = None
                self.await_use_prompt_turns = 0
                return self._record_decision(slot, f"pending_use_{cmd}_slot_{slot}")
            # Detect success/failure and clear state.
            if (
                "you drank the" in msg_lower
                or "you read the" in msg_lower
                or "you can't drink" in msg_lower
                or "you can't read" in msg_lower
                or "slips from your fingers" in msg_lower
                or "please select a valid item" in msg_lower
                or self.await_use_prompt_turns == 0
            ):
                self.pending_use_slot = None
                self.pending_use_cmd = None
                self.await_use_prompt_turns = 0

        # Evaluate picked-up gear: learn unknowns first, then prefer upgrades.
        if self.pending_pickup_equip_slot and not self.pending_wield_slot:
            pickup_slot = self.pending_pickup_equip_slot
            self.pending_pickup_equip_slot = None
            item_name = self._inventory_name_for_slot(state.inventory, pickup_slot)
            if item_name is None:
                item_name = f"slot_{pickup_slot}"

            should_wield = self._should_equip_item(item_name, state.equipment)

            if should_wield:
                self.mode = "equip"
                self.last_equip_item_name = item_name
                self.last_equip_baseline_ac = state.player_ac
                self.last_equip_baseline_damage = state.damage_dice
                self.wield_cooldown = 1
                self.wield_attempt_counts[item_name] = self.wield_attempt_counts.get(item_name, 0) + 1
                self.pending_wield_slot = pickup_slot
                self.await_wield_prompt_turns = 3
                return self._record_decision("w", f"inventory_changed_wield_slot_{pickup_slot}")

        # After testing an item, swap back to the known-best for that slot.
        if self.pending_reequip_kind and not self.pending_wield_slot:
            kind = self.pending_reequip_kind
            self.pending_reequip_kind = None
            best = self.item_knowledge.get("best_by_slot", {}).get(kind)
            if isinstance(best, dict):
                best_name = best.get("name", "")
                for slot, name in state.inventory or []:
                    if name.lower() == best_name.lower():
                        self.mode = "equip"
                        self.last_equip_item_name = name
                        self.last_equip_baseline_ac = state.player_ac
                        self.last_equip_baseline_damage = state.damage_dice
                        self.wield_cooldown = 1
                        self.pending_wield_slot = slot
                        self.await_wield_prompt_turns = 3
                        return self._record_decision("w", f"reequip_best_{kind}_slot_{slot}")

        # If we queued a multi-key action (e.g., open + direction), send it next.
        if self.pending_keys:
            action = self.pending_keys.pop(0)
            return self._record_decision(action, "pending_open_direction")

        if "you have picked the lock" in msg_lower:
            self.door_knowledge["lock_success"] = int(self.door_knowledge.get("lock_success", 0)) + 1
            self.knowledge_dirty = True
            if self.current_motion_pos is not None and self.last_open_dir in "hjklyubn":
                failed = self.failed_door_dirs_by_world.get(self.current_motion_pos)
                if failed and self.last_open_dir in failed:
                    failed.discard(self.last_open_dir)
                    if not failed:
                        self.failed_door_dirs_by_world.pop(self.current_motion_pos, None)
        elif "you failed to pick the lock" in msg_lower:
            self.door_knowledge["lock_fail"] = int(self.door_knowledge.get("lock_fail", 0)) + 1
            self.knowledge_dirty = True
            if self.current_motion_pos is not None and self.last_open_dir in "hjklyubn":
                failed = self.failed_door_dirs_by_world.setdefault(self.current_motion_pos, set())
                failed.add(self.last_open_dir)

        if "you are now wielding the" in msg_lower:
            self.has_wielded_weapon = True
            m_wield = re.search(r"you are now wielding the\s+(.+?)\.?$", msg_lower)
            if m_wield:
                self.current_wielded_weapon = m_wield.group(1).strip()
                self.weapon_combat_turn = 0
        if "you were wielding the" in msg_lower:
            self.has_wielded_weapon = False
            self.current_wielded_weapon = None

        # Detect wielded weapon from equipment panel when not tracked via message.
        if self.current_wielded_weapon is None and state.equipment:
            for _slot, ename in state.equipment:
                if self._gear_slot_kind(ename) == "weapon":
                    self.current_wielded_weapon = ename.lower()
                    break

        # Opened doors are treated as normal walkable floor tiles.

        if "bumped into a door" in msg_lower:
            self.mode = "seek"
            self.wall_bump_chain = 0
            direction = self.last_action if self.last_action in "hjklyubn" else "h"
            self.last_open_dir = direction
            self.follow_open_dir_turns = 0
            self.pending_keys = [direction]
            return self._record_decision("o", f"open_door_then_{direction}")

        # If we bumped into a wall, mark it, but don't preempt all higher-priority logic.
        # Wall bump: attack adjacent monsters; switch to wall-follow on first wall hit.
        if "bumped into a wall" in msg_lower:
            if adjacent is not None:
                self.mode = "combat"
                dr = adjacent[0] - pos[0]
                dc = adjacent[1] - pos[1]
                action = pf.DIR_TO_KEY.get((dr, dc), ".")
                if action in "hjklyubn":
                    return self._record_decision(action, "corner_breakout_attack")
            if self.explore_phase == "head_east":
                self.explore_phase = "wall_follow"
                # Start heading south (clockwise turn from east).
                self.wall_follow_heading = "j"
                self.wall_follow_start_wpos = motion_pos
                self.wall_follow_started = False

        # 1) Survival first: rest when HP below 50% and no visible threats.
        if state.player_max_hp > 0 and state.hp_pct < 0.50:
            self.mode = "recover"
            adjacent = None
            if self._monster_signal_reliable(state):
                adjacent = self._adjacent_monster(pos, state.monsters)
            if adjacent is None and not state.monsters:
                return self._record_decision("R", "low_hp_rest_no_visible_threat")
            if adjacent is None and state.monsters:
                # Monster visible but not adjacent — keep fleeing, don't rest.
                flee_target = state.monsters[0][:2]
                flee, flee_mode = self._brave_flee_key(state, pos, flee_target, allow_equal=False)
                if flee:
                    return self._record_decision(flee, f"low_hp_flee_visible_{flee_mode}")
            if adjacent is not None:
                flee, flee_mode = self._brave_flee_key(state, pos, adjacent, allow_equal=False)
                if flee:
                    return self._record_decision(flee, f"low_hp_flee_{flee_mode}_from_{adjacent}")

        # 2) Consumable usage: quaff healing when hurt, try unknowns when safe.
        use_action = self._consider_consumable_use(state, adjacent)
        if use_action:
            return use_action

        # 3) No background wield-cycling: wield is pickup-driven to preserve movement.

        # 4) Immediate combat: bump-attack adjacent monster.
        # Skip phantom monsters: if stuck attacking same spot with no combat feedback.
        if adjacent is not None and self.stuck_turns >= 4 and self.turns_since_combat_feedback >= 4:
            phantom_wp = self._local_to_world(state, adjacent)
            if phantom_wp:
                self.phantom_positions.add(phantom_wp)
            adjacent = None  # treat as phantom, fall through to exploration

        if adjacent is not None:
            self.mode = "combat"
            if state.player_hp <= 1:
                flee, flee_mode = self._brave_flee_key(state, pos, adjacent, allow_equal=True)
                if flee and flee in "hjklyubn":
                    self.mode = "recover"
                    return self._record_decision(flee, f"fragile_hp_flee_{flee_mode}")
            threat_name = self.recent_attacker_name
            danger = self._monster_danger_score(threat_name) if threat_name else 0.0
            confidence = self._monster_confidence(threat_name) if threat_name else 0.0
            if self._should_flee_known_threat(state, danger, confidence):
                flee, flee_mode = self._brave_flee_key(state, pos, adjacent, allow_equal=True)
                if flee:
                    return self._record_decision(
                        flee,
                        f"flee_known_threat_{flee_mode}_{threat_name}_d{danger:.2f}_c{confidence:.2f}",
                    )
            # Flee from unknown monsters when not healthy enough to risk it.
            glyph = self._adjacent_monster_glyph(pos, state.monsters)
            threat_lvl = self._glyph_threat_level(glyph)
            if confidence < 0.3:
                # Major monster (uppercase glyph) — flee unless nearly full HP.
                if threat_lvl >= 2 and state.hp_pct < 0.80:
                    flee, flee_mode = self._brave_flee_key(state, pos, adjacent, allow_equal=True)
                    if flee:
                        return self._record_decision(
                            flee,
                            f"flee_unknown_major_{flee_mode}_glyph_{glyph}_hp{state.hp_pct:.0%}",
                        )
                # Any unknown monster — flee if HP is below 60%.
                if state.hp_pct < 0.60:
                    flee, flee_mode = self._brave_flee_key(state, pos, adjacent, allow_equal=True)
                    if flee:
                        return self._record_decision(
                            flee,
                            f"flee_unknown_{flee_mode}_glyph_{glyph}_hp{state.hp_pct:.0%}",
                        )
            dr = adjacent[0] - pos[0]
            dc = adjacent[1] - pos[1]
            action = pf.DIR_TO_KEY.get((dr, dc), ".")
            return self._record_decision(
                action,
                f"{self._goal_thought(state, 'adjacent_attack', adjacent)}_d{danger:.2f}_c{confidence:.2f}",
            )

        # Post-combat rest: recover to 70% HP before exploring further.
        if state.player_max_hp > 0 and state.hp_pct < 0.70 and self.recent_attacker_name and not state.monsters:
            self.mode = "recover"
            return self._record_decision("R", f"post_combat_rest_hp{state.hp_pct:.0%}")

        # === EXPLORATION: Wall hug ===
        self.mode = "seek"

        # Phase 1: head east until a wall bump switches us.
        if self.explore_phase == "head_east":
            if self._can_step(state.map, pos, "l"):
                return self._record_decision("l", "head_east")
            # Can't step east but no bump message yet — wait a tick.
            return self._record_decision("l", "head_east_push")

        # Phase 2: clockwise wall follow (right-hand rule, 8 directions).
        # Lap check: if we loop back to start, search for secret doors then continue.
        if (
            self.wall_follow_started
            and self.wall_follow_start_wpos is not None
            and motion_pos == self.wall_follow_start_wpos
        ):
            # Reset for another lap — keep wall-following.
            self.wall_follow_start_wpos = motion_pos
            self.wall_follow_started = False
            return self._record_decision("s", "wall_follow_lap_search")

        step = self._wall_follow_cw(state.map, pos)
        if step:
            # If the step leads to a closed door, open it first.
            target = self._step_pos(pos, step)
            if target and state.map[target[0]][target[1]] == '+':
                self.last_open_dir = step
                self.pending_keys = [step]
                return self._record_decision("o", f"wall_hug_open_{step}")
            # Mark that we've moved away from start.
            if not self.wall_follow_started and motion_pos != self.wall_follow_start_wpos:
                self.wall_follow_started = True
            return self._record_decision(step, f"wall_hug_{step}")

        # Fallback: try any wall-adjacent direction.
        for key in "ljkhyubn":
            if self._can_step(state.map, pos, key):
                return self._record_decision(key, f"stuck_fallback_{key}")

        self.mode = "idle"
        return self._record_decision(".", "idle_wait")

    def _key_toward(self, grid, start, goal):
        path = pf.path_to(grid, start, goal)
        key = pf.first_step_key(path)
        return self._sanitize_move(key)

    def _local_to_world_pos(self, local_pos):
        """Convert a local grid position to world position using current state offsets."""
        if (
            self.current_motion_pos is None
            or self.current_pos is None
        ):
            return None
        wx, wy = self.current_motion_pos
        pr, pc = self.current_pos
        lr, lc = local_pos
        return (wx + (lc - pc), wy + (lr - pr))

    @staticmethod
    def _world_to_local(state, world_pos):
        if (
            world_pos is None
            or state.player_world_pos is None
            or state.player_pos is None
            or not state.map
        ):
            return None
        wx, wy = state.player_world_pos
        tx, ty = world_pos
        pr, pc = state.player_pos
        lr = pr + (ty - wy)
        lc = pc + (tx - wx)
        if lr < 0 or lc < 0 or lr >= len(state.map) or lc >= len(state.map[0]):
            return None
        return (lr, lc)

    @staticmethod
    def _raw_key_toward(grid, start, goal):
        path = pf.path_to(grid, start, goal)
        return pf.first_step_key(path)

    def _brave_flee_key(self, state, pos, threat_pos, allow_equal=False):
        base_dist = pf.heuristic(pos, threat_pos)

        door_goal = self._nearest_door_approach(state.map, pos)
        if door_goal:
            k = self._raw_key_toward(state.map, pos, door_goal)
            if self._flee_key_is_safe(state.map, pos, threat_pos, base_dist, k, allow_equal):
                return k, "door"

        hallway_goal = self._nearest_hallway_target(state.map, pos)
        if hallway_goal:
            k = self._raw_key_toward(state.map, pos, hallway_goal)
            if self._flee_key_is_safe(state.map, pos, threat_pos, base_dist, k, allow_equal):
                return k, "hall"

        flee = pf.key_away_from(pos, threat_pos, state.map)
        if self._flee_key_is_safe(state.map, pos, threat_pos, base_dist, flee, allow_equal):
            return flee, "away"
        return flee, "away_risky"

    @staticmethod
    def _flee_key_is_safe(grid, pos, threat_pos, base_dist, key, allow_equal):
        if key not in KEY_TO_DIR:
            return False
        dr, dc = KEY_TO_DIR[key]
        nr, nc = pos[0] + dr, pos[1] + dc
        if nr < 0 or nc < 0 or nr >= len(grid) or nc >= len(grid[0]):
            return False
        if not pf.is_walkable(grid[nr][nc]):
            return False
        new_dist = pf.heuristic((nr, nc), threat_pos)
        return new_dist >= base_dist if allow_equal else new_dist > base_dist

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
            f"mode={self.mode} "
            f"phase={self.explore_phase} "
            f"heading={self.wall_follow_heading} "
            f"stuck={self.stuck_turns}"
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

    @staticmethod
    def _local_to_world(state, local_pos):
        if state.player_world_pos is None or state.player_pos is None:
            return None
        pr, pc = state.player_pos
        lr, lc = local_pos
        wx, wy = state.player_world_pos
        return (wx + (lc - pc), wy + (lr - pr))

    def _is_item_blacklisted(self, state, local_pos):
        wp = self._local_to_world(state, local_pos)
        if wp is None:
            return False
        return self.failed_item_goals.get(wp, 0) >= 3

    def _sanitize_move(self, key):
        if key is None:
            return None
        if key not in "hjklyubn":
            return key

        if self.avoid_key_turns > 0 and self.avoid_key in "hjklyubn" and key == self.avoid_key:
            for alt in "hjklyubn":
                if alt == key:
                    continue
                if self._is_blocked_dir(alt):
                    continue
                return alt

        if self._is_blocked_dir(key):
            for alt in "hjklyubn":
                if alt != key and not self._is_blocked_dir(alt):
                    key = alt
                    break

        if not self.action_history:
            return key

        # Avoid immediate backtracking oscillation unless we are in clear trouble.
        prev = self.action_history[-1]
        if self._is_dead_end_tile(self.current_map, self.current_pos):
            return key
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
            # Reset cycle escalation when reaching genuinely new territory.
            if progressed and world not in set(list(self.world_history)[:-1]):
                self.cycle_escalation = 0
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

    # Clockwise direction ordering for 8-way movement.
    _CW = ['k', 'u', 'l', 'n', 'j', 'b', 'h', 'y']
    _CW_IDX = {k: i for i, k in enumerate(_CW)}

    def _wall_follow_cw(self, grid, pos):
        """Clockwise wall-follow using left-hand rule with 8 directions.

        Wall stays on the LEFT. Check left first so the bot turns into
        hallways and doors on the wall side instead of walking past them.
        Only steps onto tiles adjacent to at least one wall.
        Treats closed doors as valid steps (caller handles opening).
        """
        heading = self.wall_follow_heading
        if heading is None or heading not in self._CW_IDX:
            heading = 'j'
        h_idx = self._CW_IDX[heading]

        # Left-hand rule for clockwise traversal (wall on left):
        # 90° left, 45° left, straight, 45° right, 90° right, 135° right,
        # 135° left, 180° back.
        for offset in (-2, -1, 0, 1, 2, 3, -3, 4):
            cand = self._CW[((h_idx + offset) % 8)]
            if not self._can_step_or_door(grid, pos, cand):
                continue
            target = self._step_pos(pos, cand)
            if target and self._adjacent_to_wall(grid, target):
                self.wall_follow_heading = cand
                return cand

        return None

    @staticmethod
    def _adjacent_to_wall(grid, pos):
        """Return True if pos is next to at least one wall/solid tile."""
        r, c = pos
        rows = len(grid)
        cols = len(grid[0]) if rows else 0
        for dr in (-1, 0, 1):
            for dc in (-1, 0, 1):
                if dr == 0 and dc == 0:
                    continue
                nr, nc = r + dr, c + dc
                if nr < 0 or nc < 0 or nr >= rows or nc >= cols:
                    continue
                ch = grid[nr][nc]
                if ch in ('#', ':', '+', ' '):
                    return True
        return False

    @staticmethod
    def _can_step(grid, pos, key):
        if not grid or pos is None or key not in KEY_TO_DIR:
            return False
        pr, pc = pos
        dr, dc = KEY_TO_DIR[key]
        nr, nc = pr + dr, pc + dc
        if nr < 0 or nc < 0 or nr >= len(grid) or nc >= len(grid[0]):
            return False
        return pf.is_walkable(grid[nr][nc])

    @staticmethod
    def _can_step_or_door(grid, pos, key):
        """Like _can_step but also returns True for closed doors."""
        if not grid or pos is None or key not in KEY_TO_DIR:
            return False
        pr, pc = pos
        dr, dc = KEY_TO_DIR[key]
        nr, nc = pr + dr, pc + dc
        if nr < 0 or nc < 0 or nr >= len(grid) or nc >= len(grid[0]):
            return False
        ch = grid[nr][nc]
        return pf.is_walkable(ch) or ch == '+'

    @staticmethod
    def _step_pos(pos, key):
        """Return the grid position one step in direction key from pos."""
        if pos is None or key not in KEY_TO_DIR:
            return None
        pr, pc = pos
        dr, dc = KEY_TO_DIR[key]
        return (pr + dr, pc + dc)

    @staticmethod
    def _adjacent_door_direction(grid, pos, avoid_dirs=None):
        pr, pc = pos
        avoid = avoid_dirs or set()
        for key in "hjklyubn":
            if key in avoid:
                continue
            dr, dc = KEY_TO_DIR[key]
            nr, nc = pr + dr, pc + dc
            if nr < 0 or nc < 0 or nr >= len(grid) or nc >= len(grid[0]):
                continue
            if grid[nr][nc] == "+":
                return key
        return None

    def _should_prioritize_door_hunt(self, state, pos):
        # Trigger when movement progress is poor in open room-like spaces.
        if not state.map or pos is None:
            return False
        if self.no_progress_turns < 2 and self.stuck_turns < 2 and self.wall_bump_chain < 2:
            return False
        return self._is_open_roomish(state.map, pos)

    @staticmethod
    def _is_open_roomish(grid, pos):
        rows = len(grid)
        cols = len(grid[0]) if rows else 0
        pr, pc = pos
        walkable_neighbors = 0
        for dr in (-1, 0, 1):
            for dc in (-1, 0, 1):
                if dr == 0 and dc == 0:
                    continue
                nr, nc = pr + dr, pc + dc
                if nr < 0 or nc < 0 or nr >= rows or nc >= cols:
                    continue
                if pf.is_walkable(grid[nr][nc]):
                    walkable_neighbors += 1
        # Open-room proxy: many nearby walkable tiles.
        return walkable_neighbors >= 6

    @staticmethod
    def _is_dead_end_tile(grid, pos):
        if not grid or pos is None:
            return False
        rows = len(grid)
        cols = len(grid[0]) if rows else 0
        pr, pc = pos
        orth = [(-1, 0), (1, 0), (0, -1), (0, 1)]
        walkable_orth = 0
        for dr, dc in orth:
            nr, nc = pr + dr, pc + dc
            if nr < 0 or nc < 0 or nr >= rows or nc >= cols:
                continue
            if pf.is_walkable(grid[nr][nc]):
                walkable_orth += 1
        return walkable_orth <= 1

    @staticmethod
    def _opposite_key(key):
        opposites = {
            "h": "l", "l": "h",
            "j": "k", "k": "j",
            "y": "n", "n": "y",
            "u": "b", "b": "u",
        }
        return opposites.get(key)

    @staticmethod
    def _is_hallway_tile(grid, pos):
        if not grid or pos is None:
            return False
        rows = len(grid)
        cols = len(grid[0]) if rows else 0
        pr, pc = pos
        orth = [(-1, 0), (1, 0), (0, -1), (0, 1)]
        walkable_orth = 0
        for dr, dc in orth:
            nr, nc = pr + dr, pc + dc
            if nr < 0 or nc < 0 or nr >= rows or nc >= cols:
                continue
            if pf.is_walkable(grid[nr][nc]):
                walkable_orth += 1
        return walkable_orth <= 2

    def _nearest_door_approach(self, grid, pos):
        return pf.find_nearest_target(
            grid,
            pos,
            lambda ch, p: pf.is_walkable(ch)
            and p != pos
            and self._adjacent_door_direction(grid, p) is not None,
        )

    def _nearest_hallway_target(self, grid, pos):
        return pf.find_nearest_target(
            grid,
            pos,
            lambda ch, p: pf.is_walkable(ch)
            and p != pos
            and self._is_hallway_tile(grid, p),
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

    def _adjacent_monster_filtered(self, state, pos, monsters):
        """Like _adjacent_monster but skips known phantom positions."""
        pr, pc = pos
        for mr, mc, _ in monsters:
            if abs(mr - pr) <= 1 and abs(mc - pc) <= 1 and (mr, mc) != (pr, pc):
                wp = self._local_to_world(state, (mr, mc))
                if wp and wp in self.phantom_positions:
                    continue
                return (mr, mc)
        return None

    @staticmethod
    def _adjacent_monster_glyph(pos, monsters):
        """Return the glyph character of the nearest adjacent monster, or None."""
        pr, pc = pos
        for mr, mc, ch in monsters:
            if abs(mr - pr) <= 1 and abs(mc - pc) <= 1 and (mr, mc) != (pr, pc):
                return ch
        return None

    @staticmethod
    def _glyph_threat_level(glyph):
        """Estimate monster threat from glyph: uppercase = major (2), lowercase = minor (1), else 0."""
        if glyph and glyph.isupper():
            return 2
        if glyph and glyph.islower():
            return 1
        return 0

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
            if self._is_known_non_wieldable(name):
                continue
            if self.wield_attempt_counts.get(name, 0) >= 3:
                continue

            kind = self._gear_slot_kind(name)

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
        """Score an item by learned observations, with a small heuristic fallback."""
        n = item_name.lower()
        kind = self._gear_slot_kind(item_name)
        gear_strength = self.item_knowledge.get("gear_strength", {})
        learned = gear_strength.get(n, {}) if isinstance(gear_strength, dict) else {}

        score = 0.0

        # Light sources always get a fixed high score (not combat items).
        if any(k in n for k in ("torch", "lantern")):
            return 12.0

        has_observations = isinstance(learned, dict) and (
            float(learned.get("observed_damage_avg", 0)) > 0
            or float(learned.get("observed_ac_best", 0)) > 0
        )

        if has_observations:
            # Weapon score: average damage dice observed when wielding this item.
            dmg_avg = float(learned.get("observed_damage_avg", 0))
            to_dam = float(learned.get("observed_to_dam", 0))
            score += (dmg_avg + to_dam) * 1.5

            # Armor/shield score: AC contribution observed on equip.
            ac_best = float(learned.get("observed_ac_best", 0))
            ac_avg = float(learned.get("observed_ac_avg", 0))
            score += ac_best * 3.0
            score += ac_avg * 1.2

            # Small credit for reliability (successfully equipped).
            score += min(0.8, float(learned.get("successes", 0)) * 0.1)

        return score

    @staticmethod
    def _inventory_name_for_slot(inventory, slot):
        for s, name in inventory or []:
            if s == slot:
                return name
        return None

    def _equipped_name_for_kind(self, equipment, kind):
        """Return the name of the currently equipped item in a given slot kind."""
        for _slot, name in equipment or []:
            if self._gear_slot_kind(name) == kind:
                return name
        return None

    def _is_gear_tested(self, item_name):
        """True if we have real observations for this item from a prior wield."""
        n = item_name.lower()
        gear_strength = self.item_knowledge.get("gear_strength", {})
        learned = gear_strength.get(n, {}) if isinstance(gear_strength, dict) else {}
        if not isinstance(learned, dict):
            return False
        return (
            float(learned.get("observed_damage_avg", 0)) > 0
            or float(learned.get("observed_ac_best", 0)) > 0
            or int(learned.get("successes", 0)) > 0
        )

    def _should_equip_item(self, item_name, equipment):
        """Decide whether to wield a picked-up item.

        Priority: learn unknowns first, then maximize score.
        - Untested gear item → always try it (knowledge is permanent).
        - Tested and strictly better than equipped → upgrade.
        - Otherwise → keep current gear.
        """
        kind = self._gear_slot_kind(item_name)
        if not kind:
            # Not recognizable gear — try it once to find out.
            return self.wield_attempt_counts.get(item_name, 0) == 0

        # If we've never tested this item, wield it to learn its stats.
        if not self._is_gear_tested(item_name):
            return True

        # Both items are known — compare scores.
        equipped_name = self._equipped_name_for_kind(equipment, kind)
        if not equipped_name:
            # Nothing equipped in this slot — wield it.
            return True

        new_score = self._gear_score(item_name)
        equipped_score = self._gear_score(equipped_name)
        return new_score > equipped_score

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

        # Successful equip: learn AC and damage impact, track best-known item per slot.
        m = re.search(r"you are now wielding the\s+(.+?)\.?$", message, flags=re.IGNORECASE)
        if m:
            item = m.group(1).strip()
            if item:
                self._learn_successful_equip(item, state.player_ac, state.damage_dice, state.to_hit_bonus, state.to_dam_bonus)
                # After learning, check if a better item for this slot is in inventory.
                kind = self._gear_slot_kind(item)
                if kind:
                    best = self.item_knowledge.get("best_by_slot", {}).get(kind)
                    if isinstance(best, dict) and best.get("name", "").lower() != item.lower():
                        self.pending_reequip_kind = kind

    @staticmethod
    def _dice_avg(dice_str):
        """Compute the average roll for NdM notation, e.g. '2d8' -> 9.0."""
        if not dice_str:
            return 0.0
        m = re.match(r"(\d+)d(\d+)", dice_str.strip())
        if not m:
            return 0.0
        n, sides = int(m.group(1)), int(m.group(2))
        return n * (sides + 1) / 2.0

    def _learn_successful_equip(self, item_name, current_ac, current_damage="", to_hit=0, to_dam=0):
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
                "observed_damage": "",
                "observed_damage_avg": 0.0,
                "observed_to_hit": 0,
                "observed_to_dam": 0,
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

        # Record the damage dice the game reports while wielding this weapon.
        if current_damage:
            entry["observed_damage"] = current_damage
            entry["observed_damage_avg"] = self._dice_avg(current_damage)
        if to_hit:
            entry["observed_to_hit"] = int(to_hit)
        if to_dam:
            entry["observed_to_dam"] = int(to_dam)

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
        self.last_equip_baseline_damage = ""

    def _queue_pickup_equip_from_message(self, state):
        """On pickup message, queue wield only if item isn't known non-wieldable."""
        msg = (state.last_message or "").strip()
        if not msg:
            return
        m = re.search(r"you have an?\s+(.+?)\.?$", msg, flags=re.IGNORECASE)
        if not m:
            return
        picked = m.group(1).strip().lower()
        if not picked:
            return

        # Don't try to wield items we know can't be wielded.
        if self._is_known_non_wieldable(picked):
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
            # Don't try to wield items we know can't be wielded.
            if self._is_known_non_wieldable(name):
                continue
            self.pending_pickup_equip_slot = slot
            return

    @staticmethod
    def _item_category(item_name):
        """Extract the general category from an item name.

        'Blue Potion' -> 'potion', 'Scroll labeled foo' -> 'scroll',
        'Yew Wand' -> 'wand', 'Dagger' -> 'dagger'.
        """
        n = item_name.strip().lower()
        if n.startswith("scroll labeled") or n.startswith("scroll"):
            return "scroll"
        # For "<adjective> <noun>" items, the last word is the category.
        parts = n.split()
        return parts[-1] if parts else n

    def _learn_non_wieldable(self, item_name):
        cat = self._item_category(item_name)
        if cat and cat not in self.learned_non_wieldable_categories:
            self.learned_non_wieldable_categories.add(cat)
            self.knowledge_dirty = True

    def _learn_from_monster_feedback(self, message, hp_loss=0):
        if not message:
            return

        self.turns_since_combat_feedback += 1
        msg = message.strip()

        # You hit the Giant Ant.
        m = re.search(r"you hit the\s+(.+?)\.?$", msg, flags=re.IGNORECASE)
        if m:
            self.turns_since_combat_feedback = 0
            self._monster_note(m.group(1), "hits", 1)
            self._weapon_combat_note("hits")
            return

        # You miss the Giant Ant.
        m = re.search(r"you miss the\s+(.+?)\.?$", msg, flags=re.IGNORECASE)
        if m:
            self.turns_since_combat_feedback = 0
            self._monster_note(m.group(1), "misses", 1)
            self._weapon_combat_note("misses")
            return

        # You have slain the Giant Ant.
        m = re.search(r"you have slain the\s+(.+?)\.?$", msg, flags=re.IGNORECASE)
        if m:
            self.turns_since_combat_feedback = 0
            self._monster_note(m.group(1), "kills", 1)
            self._weapon_combat_note("kills")
            return

        # The Giant Ant bites/touches/claws/breathes ...
        m = re.search(
            r"the\s+(.+?)\s+(bites|claws|touches|hits|breathes|stings|kicks|gazes|spits)\b",
            msg,
            flags=re.IGNORECASE,
        )
        if m:
            self.turns_since_combat_feedback = 0
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

    def _weapon_combat_note(self, event):
        """Track combat events (hits/misses/kills) for the currently wielded weapon."""
        weapon = self.current_wielded_weapon
        if not weapon:
            return
        n = weapon.lower()
        gear_strength = self.item_knowledge.setdefault("gear_strength", {})
        entry = gear_strength.setdefault(
            n,
            {
                "successes": 0,
                "observed_ac_total": 0,
                "observed_ac_count": 0,
                "observed_ac_avg": 0.0,
                "observed_ac_best": 0,
                "observed_damage": "",
                "observed_damage_avg": 0.0,
                "observed_to_hit": 0,
                "observed_to_dam": 0,
                "combat_hits": 0,
                "combat_misses": 0,
                "combat_kills": 0,
                "combat_turns": 0,
            },
        )
        if event == "hits":
            entry["combat_hits"] = int(entry.get("combat_hits", 0)) + 1
        elif event == "misses":
            entry["combat_misses"] = int(entry.get("combat_misses", 0)) + 1
        elif event == "kills":
            entry["combat_kills"] = int(entry.get("combat_kills", 0)) + 1
        entry["combat_turns"] = int(entry.get("combat_turns", 0)) + 1
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

    # -- Consumable observation system --
    # Learns from screen messages and stat changes, not from source code.

    def _learn_from_consumable_feedback(self, state):
        """Track effects after quaffing or reading by observing screen messages and HP changes."""
        msg = (state.last_message or "").strip()
        if not msg:
            return

        lower = msg.lower()

        # Detect quaff event: "You drank the <flavor name>."
        m = re.search(r"you drank the\s+(.+?)\.?$", lower)
        if m:
            flavor = m.group(1).strip()
            if flavor:
                self.pending_consumable_flavor = flavor
                self.pre_use_hp = self.last_player_hp
                self.use_cooldown = 2
            return

        # Detect read event: "You read the <name>."
        m = re.search(r"you read the\s+(.+?)\.?$", lower)
        if m:
            flavor = m.group(1).strip()
            if flavor:
                self.pending_consumable_flavor = flavor
                self.pre_use_hp = self.last_player_hp
                self.use_cooldown = 2
            return

        # Detect failure messages and clear state.
        if "you can't drink" in lower or "you can't read" in lower:
            self.pending_consumable_flavor = None
            self.pre_use_hp = None
            return

        if not self.pending_consumable_flavor:
            return

        # Skip prompts that aren't effect messages.
        if any(k in lower for k in ("choose an item", "quaff which", "read which", "wield which")):
            return

        # Identity reveal: "You have no more Orange Potions of Cure Light Wounds"
        m = re.search(r"you have no more\s+(.+?)\s+of\s+(.+?)\.?$", lower)
        if m:
            flavor_part = m.group(1).strip()
            true_identity = m.group(2).strip()
            if true_identity:
                self._learn_consumable_identity(self.pending_consumable_flavor, true_identity, state)
            self.pending_consumable_flavor = None
            self.pre_use_hp = None
            return

        # Tried but no effect: "You have no more Blue Potions {tried}"
        if "{tried}" in lower:
            self._learn_consumable_tried(self.pending_consumable_flavor)
            self.pending_consumable_flavor = None
            self.pre_use_hp = None
            return

        # Any other message after use is an observed effect — record the raw text
        # and any HP change as evidence of what this flavor does.
        hp_delta = 0
        if self.pre_use_hp is not None and state.player_hp is not None:
            hp_delta = state.player_hp - self.pre_use_hp

        self._learn_consumable_observation(self.pending_consumable_flavor, msg, hp_delta)
        self.pending_consumable_flavor = None
        self.pre_use_hp = None

    def _learn_consumable_identity(self, flavor_name, true_identity, state):
        """The game revealed the true name of a consumable (e.g., 'Cure Light Wounds')."""
        flavor = flavor_name.lower()
        identity = true_identity.lower()
        self.flavor_map[flavor] = identity

        entry = self.consumable_knowledge.setdefault(identity, {
            "count": 0, "hp_delta_total": 0, "observations": [],
        })
        entry["count"] = int(entry.get("count", 0)) + 1
        entry["identified"] = True
        if self.pre_use_hp is not None and state.player_hp is not None:
            hp_delta = state.player_hp - self.pre_use_hp
            entry["hp_delta_total"] = int(entry.get("hp_delta_total", 0)) + hp_delta
            if hp_delta > 0:
                entry["heals"] = True
            elif hp_delta < 0:
                entry["harms"] = True
        self.knowledge_dirty = True

    def _learn_consumable_tried(self, flavor_name):
        """Consumable had no visible effect — mark as tried."""
        flavor = flavor_name.lower()
        if flavor not in self.flavor_map:
            self.flavor_map[flavor] = "{tried}"
        self.knowledge_dirty = True

    def _learn_consumable_observation(self, flavor_name, raw_msg, hp_delta):
        """Record a raw screen observation after consuming something."""
        flavor = flavor_name.lower()

        # Use HP delta as primary evidence.
        effect_tag = "unknown"
        if hp_delta > 0:
            effect_tag = "healed"
        elif hp_delta < 0:
            effect_tag = "harmed"

        self.flavor_map[flavor] = effect_tag

        entry = self.consumable_knowledge.setdefault(effect_tag + ":" + flavor, {
            "count": 0, "hp_delta_total": 0, "observations": [],
        })
        entry["count"] = int(entry.get("count", 0)) + 1
        entry["hp_delta_total"] = int(entry.get("hp_delta_total", 0)) + hp_delta
        if hp_delta > 0:
            entry["heals"] = True
        elif hp_delta < 0:
            entry["harms"] = True
        obs = entry.setdefault("observations", [])
        if raw_msg and raw_msg not in obs and len(obs) < 5:
            obs.append(raw_msg)
        self.knowledge_dirty = True

    def _is_known_healing_flavor(self, item_name):
        """Check if we've observed this flavor healing us in the current run."""
        effect = self.flavor_map.get(item_name.lower(), "")
        if effect == "healed":
            return True
        # Also match if the revealed identity contains healing keywords.
        if effect and effect not in ("{tried}", "harmed", "unknown"):
            entry = self.consumable_knowledge.get(effect, {})
            return entry.get("heals", False)
        return False

    def _is_known_bad_flavor(self, item_name):
        """Check if we've observed this flavor harming us in the current run."""
        effect = self.flavor_map.get(item_name.lower(), "")
        if effect == "harmed":
            return True
        if effect and effect not in ("{tried}", "healed", "unknown"):
            entry = self.consumable_knowledge.get(effect, {})
            return entry.get("harms", False)
        return False

    def _is_consumable(self, item_name):
        """Check if an item is a potion, scroll, or wand by category."""
        cat = self._item_category(item_name)
        return cat in ("potion", "scroll", "wand")

    def _find_inventory_consumable(self, inventory, category=None, exclude_bad=True):
        """Find a consumable in inventory, optionally filtering by category.

        Returns (slot_letter, item_name) or (None, None).
        """
        for slot, name in (inventory or []):
            cat = self._item_category(name)
            if category and cat != category:
                continue
            if not category and cat not in ("potion", "scroll", "wand"):
                continue
            if exclude_bad and self._is_known_bad_flavor(name):
                continue
            return slot, name
        return None, None

    def _find_healing_potion(self, inventory):
        """Find a potion known to be healing in the current run."""
        for slot, name in (inventory or []):
            if self._item_category(name) == "potion" and self._is_known_healing_flavor(name):
                return slot, name
        return None, None

    def _consider_consumable_use(self, state, adjacent):
        """Decide whether to use a consumable. Returns an action string or None."""
        if self.pending_use_slot or self.use_cooldown > 0:
            return None

        # Emergency healing: quaff known healing potion when HP is low.
        if state.player_max_hp > 0 and state.hp_pct < 0.50:
            slot, name = self._find_healing_potion(state.inventory)
            if slot:
                return self._initiate_use("q", slot, name)

        # Don't experiment with unknown consumables if a monster is adjacent.
        if adjacent is not None:
            return None

        # When safe and HP is low-ish, try an unknown potion (might be healing).
        if state.player_max_hp > 0 and state.hp_pct < 0.60:
            slot, name = self._find_inventory_consumable(state.inventory, category="potion")
            if slot and not self.flavor_map.get(name.lower()):
                return self._initiate_use("q", slot, name)

        # When safe at decent HP, try an unknown scroll to identify it.
        if state.player_max_hp > 0 and state.hp_pct > 0.70:
            slot, name = self._find_inventory_consumable(state.inventory, category="scroll")
            if slot and not self.flavor_map.get(name.lower()):
                return self._initiate_use("r", slot, name)

        return None

    def _initiate_use(self, cmd, slot, item_name):
        """Start a consumable use sequence (q/r) for the given inventory slot."""
        self.pending_use_cmd = cmd
        self.pending_use_slot = slot
        self.pending_use_item_name = item_name
        self.await_use_prompt_turns = 3
        self.use_cooldown = 3
        self.mode = "use"
        cmd_name = {"q": "quaff", "r": "read"}.get(cmd, cmd)
        return self._record_decision(cmd, f"use_{cmd_name}_{item_name}")

    def _is_known_non_wieldable(self, item_name):
        """Check if this item's category has been learned as non-wieldable."""
        cat = self._item_category(item_name)
        return cat in self.learned_non_wieldable_categories

    def load_knowledge(self, file_path):
        try:
            with open(file_path, "r", encoding="utf-8") as f:
                data = json.load(f)
            cats = data.get("non_wieldable_categories", [])
            # Migrate legacy per-name entries to categories.
            legacy_names = data.get("non_wieldable_names", [])
            monsters = data.get("monster_knowledge", {})
            scrolls = data.get("scroll_knowledge", {})
            item_k = data.get("item_knowledge", {})
            door_k = data.get("door_knowledge", {})
            map_k = data.get("map_knowledge", {})
            consumable_k = data.get("consumable_knowledge", {})
            self.learned_non_wieldable_categories.update(c.lower() for c in cats if isinstance(c, str))
            for n in legacy_names:
                if isinstance(n, str):
                    self.learned_non_wieldable_categories.add(self._item_category(n))
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
            if isinstance(consumable_k, dict):
                self.consumable_knowledge = consumable_k
            self.knowledge_dirty = False
            return True
        except FileNotFoundError:
            return False
        except Exception:
            return False

    def save_knowledge(self, file_path):
        data = {
            "schema_version": 4,
            "non_wieldable_categories": sorted(self.learned_non_wieldable_categories),
            "monster_knowledge": self.monster_knowledge,
            "scroll_knowledge": self.scroll_knowledge,
            "consumable_knowledge": self.consumable_knowledge,
            "item_knowledge": self.item_knowledge,
            "door_knowledge": self.door_knowledge,
            "map_knowledge": self.map_knowledge,
        }
        with open(file_path, "w", encoding="utf-8") as f:
            json.dump(data, f, indent=2, sort_keys=True)
            f.write("\n")
        self.knowledge_dirty = False
