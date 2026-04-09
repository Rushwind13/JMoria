"""decision.py - Decision engine for the JMoria bot."""

from collections import deque
import json
import re

from . import pathfinding as pf

KEY_TO_DIR = {v: k for k, v in pf.DIR_TO_KEY.items()}


class DecisionEngine:
    def __init__(self):
        self.visited_world = set()  # (depth, world_row, world_col)
        self.last_pos = None
        self.stuck_turns = 0
        self.last_action = "."
        self.pending_keys = []
        self.pending_wield_slot = None
        self.await_wield_prompt_turns = 0
        self.action_history = deque(maxlen=12)
        self.has_wielded_weapon = False
        self.wield_cooldown = 0
        self.equip_attempt_counts = {}
        self.wield_attempt_counts = {}
        self.learned_non_wieldable_categories = set()
        self.monster_knowledge = {}
        self.consumable_knowledge = {}
        self.flavor_map = {}  # per-run: flavor name -> observed effect
        self.pending_use_cmd = None  # "q" or "r"
        self.pending_use_slot = None
        self.pending_use_item_name = None
        self.await_use_prompt_turns = 0
        self.pending_consumable_flavor = None
        self.pre_use_snapshot = None
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
        self.map_knowledge = {"depth_notes": {}}
        self.knowledge_dirty = False
        self.last_equip_item_name = None
        self.last_equip_baseline_ac = None
        self.last_equip_baseline_damage = ""
        self.pending_pickup_equip_slot = None
        self.pending_reequip_kind = None
        self.last_inventory_entries = set()
        self.last_depth = 1
        self.last_open_dir = None
        self.last_player_hp = None
        self.recent_attacker_name = None
        self.last_thought = "idle"
        self.current_wielded_weapon = None
        self.weapon_combat_turn = 0
        self.turns_since_combat_feedback = 99
        self.phantom_positions = set()
        self.current_motion_pos = None
        self.current_map = None
        self.current_pos = None
        self.pending_scroll_label = None
        # Goal-based exploration state machine.
        self.explore_phase = "head_east"
        self.door_momentum = False
        self.known_map = [[" "] * 100 for _ in range(100)]
        self._current_depth = 1
        self.failed_door_dirs_by_world = {}
        # Door graph: nodes = door world positions, edges = same-room connectivity.
        self.door_graph = {}
        self.explored_doors = set()
        self.last_door_wpos = None
        self.cached_path = []
        self.cached_path_target = None
        # Tile-level exploration tracking.
        self.unexplored_tiles = set()
        self.explored_tiles = set()
        # Goal stack: LIFO list of (goal_type, (row, col)).
        self.goal_stack = []
        self.pushed_goals = set()
        self.failed_goals = set()
        self.last_staircase_attempt = None
        self.staircase_attempt_cooldown = 0
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
        if self.staircase_attempt_cooldown > 0:
            self.staircase_attempt_cooldown -= 1

        pos = state.player_pos
        motion_pos = state.player_world_pos if state.player_world_pos is not None else pos
        self.current_motion_pos = motion_pos
        self.current_map = state.map
        self.current_pos = pos
        self._current_depth = state.dungeon_depth
        if motion_pos is not None:
            self.visited_world.add((state.dungeon_depth, motion_pos[0], motion_pos[1]))

        self._update_known_map(state)
        self._update_door_graph(state)
        self._update_exploration_sets(state)

        hp_loss = 0
        if self.last_player_hp is not None:
            hp_loss = max(0, self.last_player_hp - state.player_hp)
        self.last_player_hp = state.player_hp

        if state.dungeon_depth != self.last_depth:
            self.equip_attempt_counts.clear()
            self.phantom_positions.clear()
            self.known_map = [[" "] * 100 for _ in range(100)]
            self.door_graph = {}
            self.explored_doors = set()
            self.last_door_wpos = None
            self.cached_path = []
            self.cached_path_target = None
            self.unexplored_tiles = set()
            self.explored_tiles = set()
            self.goal_stack = []
            self.pushed_goals = set()
            self.failed_goals = set()
            self.last_staircase_attempt = None
            self.staircase_attempt_cooldown = 0
            self.last_depth = state.dungeon_depth

        # Detect stuck behavior to break local loops.
        if self.last_pos == motion_pos:
            self.stuck_turns += 1
        else:
            self.stuck_turns = 0
        self.last_pos = motion_pos

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
            if "wield which item" in msg_lower or "choose an item from inventory" in msg_lower:
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

        # Escape lingering inventory/use prompts the bot did not initiate.
        prompt_detected = (
            ("choose an item from inventory" in msg_lower and "[" in state.last_message)
            or ("which item" in msg_lower and "[" in state.last_message)
        )
        if prompt_detected:
            if not self.pending_wield_slot and not self.pending_use_slot and not self.pending_keys:
                if self.last_action != "\x1b":
                    return self._record_decision("\x1b", "escape_stale_prompt")

        # Evaluate picked-up gear.
        if self.pending_pickup_equip_slot and not self.pending_wield_slot:
            pickup_slot = self.pending_pickup_equip_slot
            self.pending_pickup_equip_slot = None
            item_name = self._inventory_name_for_slot(state.inventory, pickup_slot)
            if item_name is None:
                item_name = f"slot_{pickup_slot}"

            if self._should_equip_item(item_name, state.equipment):
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

        # Send next key from a queued multi-key action (e.g. open + direction).
        if self.pending_keys:
            action = self.pending_keys.pop(0)
            return self._record_decision(action, "pending_open_direction")

        if "you have picked the lock" in msg_lower:
            self.door_knowledge["lock_success"] = int(self.door_knowledge.get("lock_success", 0)) + 1
            self.knowledge_dirty = True
            self.failed_goals.clear()
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

        # Detect wielded weapon from equipment panel when not captured via message.
        if self.current_wielded_weapon is None and state.equipment:
            for _slot, ename in state.equipment:
                if self._gear_slot_kind(ename) == "weapon":
                    self.current_wielded_weapon = ename.lower()
                    break

        if "bumped into a door" in msg_lower:
            self.mode = "seek"
            self.failed_goals.clear()
            direction = self.last_action if self.last_action in "hjklyubn" else "h"
            self.last_open_dir = direction
            self.pending_keys = [direction]
            return self._record_decision("o", f"open_door_then_{direction}")

        if "bumped into a wall" in msg_lower:
            if adjacent is not None:
                self.mode = "combat"
                dr = adjacent[0] - pos[0]
                dc = adjacent[1] - pos[1]
                action = pf.DIR_TO_KEY.get((dr, dc), ".")
                if action in "hjklyubn":
                    return self._record_decision(action, "corner_breakout_attack")

        # 1) Survival: rest when HP below 50% and no visible threats.
        if state.player_max_hp > 0 and state.hp_pct < 0.50:
            self.mode = "recover"
            adj_check = None
            if self._monster_signal_reliable(state):
                adj_check = self._adjacent_monster(pos, state.monsters)
            if adj_check is None and not state.monsters:
                return self._record_decision("R", "low_hp_rest_no_visible_threat")

        # 2) Consumable usage.
        use_action = self._consider_consumable_use(state, adjacent)
        if use_action:
            return use_action

        # 3) Immediate combat: bump-attack adjacent monster.
        # Skip phantom positions (stuck attacking same spot with no feedback).
        if adjacent is not None and self.stuck_turns >= 4 and self.turns_since_combat_feedback >= 4:
            phantom_wp = self._local_to_world(state, adjacent)
            if phantom_wp:
                self.phantom_positions.add(phantom_wp)
            adjacent = None

        if adjacent is not None:
            self.mode = "combat"
            dr = adjacent[0] - pos[0]
            dc = adjacent[1] - pos[1]
            action = pf.DIR_TO_KEY.get((dr, dc), ".")
            threat_name = self.recent_attacker_name
            danger = self._monster_danger_score(threat_name) if threat_name else 0.0
            confidence = self._monster_confidence(threat_name) if threat_name else 0.0
            return self._record_decision(
                action,
                f"{self._goal_thought(state, 'adjacent_attack', adjacent)}_d{danger:.2f}_c{confidence:.2f}",
            )

        # Post-combat rest: recover to 70% HP before exploring further.
        if state.player_max_hp > 0 and state.hp_pct < 0.70 and self.recent_attacker_name and not state.monsters:
            self.mode = "recover"
            return self._record_decision("R", f"post_combat_rest_hp{state.hp_pct:.0%}")

        self.mode = "seek"
        return self._pursue_goals(state, pos)

    # ------------------------------------------------------------------
    # Goal-based exploration state machine
    # ------------------------------------------------------------------

    def _pursue_goals(self, state, pos):
        """Work through the goal stack: pathfind to the top goal."""
        grid = state.map
        wpos = state.player_world_pos
        if not wpos:
            return self._record_decision(".", "no_wpos")
        mpos = self._wpos_to_rc(wpos)
        pr, pc = state.player_pos
        wx, wy = wpos

        # Check if standing on an item we have not scheduled yet.
        for ir, ic, _ich in state.items:
            item_rc = (wy + (ir - pr), wx + (ic - pc))
            if mpos == item_rc:
                key = ("item", item_rc)
                if key not in self.pushed_goals:
                    self.pushed_goals.add(key)
                    self.goal_stack.append(key)
                    break

        # After stepping through a door, push one step forward.
        if self.door_momentum:
            self.door_momentum = False
            heading = self.last_action if self.last_action in "hjklyubn" else "l"
            if self._can_step(grid, pos, heading):
                return self._record_decision(heading, "door_push_thru")

        # Initial walk east until blocked.
        if self.explore_phase == "head_east":
            if self._can_step(grid, pos, "l"):
                return self._record_decision("l", "head_east")
            self.explore_phase = "goal_stack"

        # Push new doors/items we can see.
        self._update_goal_stack(state)

        # Clean stale/completed goals from the top of the stack.
        while self.goal_stack:
            gtype, grc = self.goal_stack[-1]
            if gtype == "item":
                if mpos == grc:
                    self.goal_stack.pop()
                    continue
                break
            elif gtype == "door":
                if grc in self.explored_tiles or mpos == grc:
                    self.goal_stack.pop()
                    continue
                if not self._has_unknown_neighbor(grc):
                    self.goal_stack.pop()
                    continue
                break
            elif gtype == "unexplored":
                if grc not in self.unexplored_tiles:
                    self.goal_stack.pop()
                    continue
                if mpos == grc:
                    self.goal_stack.pop()
                    last_dir = KEY_TO_DIR.get(self.last_action)
                    check_dirs = []
                    if last_dir:
                        check_dirs.append(last_dir)
                    for d in pf.DIRS_8:
                        if d != last_dir:
                            check_dirs.append(d)
                    for dr, dc in check_dirs:
                        nr, nc = mpos[0] + dr, mpos[1] + dc
                        if (nr, nc) in self.unexplored_tiles:
                            self.goal_stack.append(("unexplored", (nr, nc)))
                            break
                    continue
                break
            elif gtype == "staircase":
                if mpos == grc:
                    can_descend, reason = self._can_safely_descend_stairs(state)
                    if can_descend:
                        self.goal_stack.pop()
                        return self._record_decision(">", reason)
                    else:
                        self.goal_stack.pop()
                        self.last_staircase_attempt = grc
                        self.staircase_attempt_cooldown = 8
                        self.last_thought = reason
                break
            else:
                self.goal_stack.pop()

        # If stack empty, push nearest unexplored tile or staircase.
        if not self.goal_stack:
            target = self._nearest_unexplored_tile(mpos)
            if target:
                self.goal_stack.append(("unexplored", target))
            elif self.staircase_attempt_cooldown <= 0:
                stair = self._find_staircase(mpos)
                if stair:
                    self.goal_stack.append(("staircase", stair))
            else:
                self.staircase_attempt_cooldown -= 1

        if not self.goal_stack:
            for key in "ljkhyubn":
                if self._can_step(grid, pos, key):
                    return self._record_decision(key, f"stuck_fallback_{key}")
            return self._record_decision(".", "idle_wait")

        gtype, grc = self.goal_stack[-1]

        # Opportunistic: if an item goal is adjacent, promote it to top.
        if gtype != "item":
            for i in range(len(self.goal_stack) - 1, -1, -1):
                if self.goal_stack[i][0] == "item":
                    irc = self.goal_stack[i][1]
                    if pf.heuristic(mpos, irc) == 1:
                        self.goal_stack.append(self.goal_stack.pop(i))
                        gtype, grc = self.goal_stack[-1]
                        self.cached_path = []
                        self.cached_path_target = None
                        break

        # Invalidate cached path if target changed.
        if self.cached_path_target != grc:
            self.cached_path = []
            self.cached_path_target = None

        # Advance cached path past current position.
        if self.cached_path:
            if self.cached_path[0] == mpos:
                self.cached_path.pop(0)
            elif mpos not in self.cached_path:
                self.cached_path = []
                self.cached_path_target = None
            else:
                while self.cached_path and self.cached_path[0] != mpos:
                    self.cached_path.pop(0)
                if self.cached_path:
                    self.cached_path.pop(0)

        # Follow cached path.
        if self.cached_path:
            nxt = self.cached_path[0]
            dkey = pf.DIR_TO_KEY.get((nxt[0] - mpos[0], nxt[1] - mpos[1]))
            if dkey:
                nxt_ch = self.known_map[nxt[0]][nxt[1]] if 0 <= nxt[0] < 100 and 0 <= nxt[1] < 100 else None
                if nxt_ch == "+":
                    self.last_open_dir = dkey
                    self.pending_keys = [dkey]
                    self.door_momentum = True
                    self.cached_path = []
                    self.cached_path_target = None
                    return self._record_decision("o", f"goal_open_{dkey}")
                if self._can_step(grid, pos, dkey):
                    return self._record_decision(dkey, f"goal_{gtype}_d{len(self.cached_path)}")
            self.cached_path = []
            self.cached_path_target = None

        # Compute new path to goal.
        dist = pf.heuristic(mpos, grc)
        if dist <= 1:
            dkey = pf.DIR_TO_KEY.get((grc[0] - mpos[0], grc[1] - mpos[1]))
            if dkey:
                tch = self.known_map[grc[0]][grc[1]]
                if tch == "+":
                    self.last_open_dir = dkey
                    self.pending_keys = [dkey]
                    self.door_momentum = True
                    return self._record_decision("o", f"goal_open_{dkey}")
                if self._can_step(grid, pos, dkey):
                    return self._record_decision(dkey, f"goal_{gtype}_step")
            self.goal_stack.pop()
            return self._pursue_goals(state, pos)

        path = pf.path_to(self.known_map, mpos, grc)
        if not path and self.known_map[grc[0]][grc[1]] == "+":
            adj = self._walkable_neighbor_of(grc)
            if adj:
                path = pf.path_to(self.known_map, mpos, adj)

        if path and len(path) >= 2:
            self.cached_path = path[1:]
            self.cached_path_target = grc
            nxt = self.cached_path[0]
            dkey = pf.DIR_TO_KEY.get((nxt[0] - mpos[0], nxt[1] - mpos[1]))
            if dkey:
                nxt_ch = self.known_map[nxt[0]][nxt[1]]
                if nxt_ch == "+":
                    self.last_open_dir = dkey
                    self.pending_keys = [dkey]
                    self.door_momentum = True
                    self.cached_path = []
                    self.cached_path_target = None
                    return self._record_decision("o", f"goal_open_{dkey}")
                if self._can_step(grid, pos, dkey):
                    return self._record_decision(dkey, f"goal_{gtype}_seek_d{dist}")

        # Pathfind failed — pop goal and retry (limited iterations).
        self.failed_goals.add(grc)
        self.goal_stack.pop()
        retry_count = 0
        while self.goal_stack and retry_count < 10:
            retry_count += 1
            gtype, grc = self.goal_stack[-1]
            dist = pf.heuristic(mpos, grc)
            if dist <= 1:
                dkey = pf.DIR_TO_KEY.get((grc[0] - mpos[0], grc[1] - mpos[1]))
                if dkey and self._can_step(grid, pos, dkey):
                    return self._record_decision(dkey, f"goal_{gtype}_step_retry")
            elif dist < 50:
                break
            self.goal_stack.pop()

        if self.goal_stack:
            self.cached_path = []
            self.cached_path_target = None
            gtype, grc = self.goal_stack[-1]
            dist = pf.heuristic(mpos, grc)
            if dist <= 1:
                dkey = pf.DIR_TO_KEY.get((grc[0] - mpos[0], grc[1] - mpos[1]))
                if dkey:
                    tch = self.known_map[grc[0]][grc[1]]
                    if tch == "+":
                        self.last_open_dir = dkey
                        self.pending_keys = [dkey]
                        self.door_momentum = True
                        return self._record_decision("o", f"goal_open_{dkey}")
                    if self._can_step(grid, pos, dkey):
                        return self._record_decision(dkey, f"goal_{gtype}_step")

        for key in "ljkhyubn":
            if self._can_step(grid, pos, key):
                return self._record_decision(key, f"stuck_fallback_{key}")
        return self._record_decision(".", "idle_wait")

    def _find_staircase(self, mpos):
        """Find nearest ">" in known_map via BFS."""
        return pf.find_nearest_target(
            self.known_map, mpos,
            lambda ch, p: ch == ">",
        )

    def _can_safely_descend_stairs(self, state):
        """
        Check descent criteria before going down (see PROGRESSION_STRATEGY.md).
        Returns: (can_descend: bool, reason: str)
        """
        if self.unexplored_tiles:
            return (False, "prog_descend_delay_exploring")

        if state.player_max_hp > 0:
            hp_pct = state.player_hp / state.player_max_hp
            if hp_pct < 0.70:
                return (False, f"prog_descend_delay_low_hp_{hp_pct:.0%}")

        if self._monster_signal_reliable(state):
            adjacent = self._adjacent_monster(state.player_pos, state.monsters)
            if adjacent:
                return (False, "prog_descend_delay_combat_adjacent")

        if self.recent_attacker_name and self.turns_since_combat_feedback < 3:
            return (False, "prog_descend_delay_combat_recent")

        return (True, "prog_descend_ready")

    # ------------------------------------------------------------------
    # Known-map management (persistent 100x100 dungeon grid)
    # ------------------------------------------------------------------

    @staticmethod
    def _wpos_to_rc(wpos):
        """Convert world pos (X, Y) to map coords (row, col) = (Y, X)."""
        return (wpos[1], wpos[0])

    def _update_known_map(self, state):
        """Blit visible screen tiles onto the persistent 100x100 known_map."""
        if not state.map or state.player_pos is None or state.player_world_pos is None:
            return
        pr, pc = state.player_pos
        wx, wy = state.player_world_pos
        monster_set = {(mr, mc) for mr, mc, _ in state.monsters}
        item_set = {(ir, ic) for ir, ic, _ in state.items}
        for lr in range(len(state.map)):
            for lc in range(len(state.map[lr])):
                ch = state.map[lr][lc]
                if ch == " ":
                    continue
                gr = wy + (lr - pr)
                gc = wx + (lc - pc)
                if 0 <= gr < 100 and 0 <= gc < 100:
                    if (lr, lc) in monster_set or (lr, lc) in item_set or ch == "@":
                        self.known_map[gr][gc] = "."
                    else:
                        self.known_map[gr][gc] = ch

        # Stamp unknown 8-neighbors of player map position as wall.
        prow, pcol = wy, wx
        for dr in (-1, 0, 1):
            for dc in (-1, 0, 1):
                if dr == 0 and dc == 0:
                    continue
                nr, nc = prow + dr, pcol + dc
                if 0 <= nr < 100 and 0 <= nc < 100:
                    if self.known_map[nr][nc] == " ":
                        self.known_map[nr][nc] = "#"

    def _update_exploration_sets(self, state):
        """Rebuild the unexplored frontier from known_map.

        A tile is 'unexplored' only if it's a passable tile in known_map
        that has at least one unknown (' ') neighbor — an exit at the
        boundary between mapped and unmapped territory.
        """
        if not state.map or state.player_pos is None or state.player_world_pos is None:
            return
        wx, wy = state.player_world_pos
        player_rc = (wy, wx)

        new_frontier = set()
        for r in range(100):
            for c in range(100):
                ch = self.known_map[r][c]
                if ch in (".", "'", "+", "<", ">"):
                    if self._has_unknown_neighbor((r, c)):
                        new_frontier.add((r, c))

        new_frontier.discard(player_rc)
        self.unexplored_tiles = new_frontier
        self.explored_tiles.add(player_rc)

    # ------------------------------------------------------------------
    # Goal stack
    # ------------------------------------------------------------------

    def _update_goal_stack(self, state):
        """Push newly-visible doors and items onto the goal stack."""
        if not state.map or state.player_pos is None or state.player_world_pos is None:
            return
        pr, pc = state.player_pos
        wx, wy = state.player_world_pos

        new_doors = []
        new_items = []

        for lr in range(len(state.map)):
            for lc in range(len(state.map[lr])):
                ch = state.map[lr][lc]
                if ch in ("'", "+"):
                    grc = (wy + (lr - pr), wx + (lc - pc))
                    if not (0 <= grc[0] < 100 and 0 <= grc[1] < 100):
                        continue
                    key = ("door", grc)
                    if key not in self.pushed_goals and grc not in self.explored_tiles:
                        self.pushed_goals.add(key)
                        new_doors.append(key)

        current_item_goals = {(t, rc) for t, rc in self.goal_stack if t == "item"}
        for ir, ic, _ich in state.items:
            grc = (wy + (ir - pr), wx + (ic - pc))
            if not (0 <= grc[0] < 100 and 0 <= grc[1] < 100):
                continue
            key = ("item", grc)
            if key not in current_item_goals:
                new_items.append(key)

        # Doors are lower priority (pushed first), items higher (pushed last).
        self.goal_stack.extend(new_doors)
        self.goal_stack.extend(new_items)

    def _has_unknown_neighbor(self, world_pos):
        """True if any 8-neighbor in known_map is unknown (" ")."""
        r, c = world_pos
        for dr in (-1, 0, 1):
            for dc in (-1, 0, 1):
                if dr == 0 and dc == 0:
                    continue
                nr, nc = r + dr, c + dc
                if 0 <= nr < 100 and 0 <= nc < 100:
                    if self.known_map[nr][nc] == " ":
                        return True
        return False

    def _walkable_neighbor_of(self, world_pos):
        """Return a walkable known_map neighbor of world_pos, or None."""
        r, c = world_pos
        for dr, dc in pf.DIRS_8:
            nr, nc = r + dr, c + dc
            if 0 <= nr < 100 and 0 <= nc < 100:
                if pf.is_walkable(self.known_map[nr][nc]):
                    return (nr, nc)
        return None

    def _nearest_unexplored_tile(self, wpos):
        """BFS on known_map to find nearest unexplored tile by walk distance."""
        if not self.unexplored_tiles:
            return None
        candidates = self.unexplored_tiles - self.failed_goals
        if not candidates:
            return None
        visited = {wpos}
        queue = deque([wpos])
        while queue:
            cur = queue.popleft()
            if cur in candidates and pf.is_walkable(self.known_map[cur[0]][cur[1]]):
                return cur
            # Check for unexplored closed doors adjacent to BFS frontier.
            for dr, dc in pf.DIRS_8:
                nr, nc = cur[0] + dr, cur[1] + dc
                if 0 <= nr < 100 and 0 <= nc < 100:
                    npos = (nr, nc)
                    if npos in candidates and self.known_map[nr][nc] == "+":
                        return npos
            # Expand BFS through walkable tiles.
            for dr, dc in pf.DIRS_8:
                nr, nc = cur[0] + dr, cur[1] + dc
                if 0 <= nr < 100 and 0 <= nc < 100:
                    npos = (nr, nc)
                    if npos not in visited and pf.is_walkable(self.known_map[nr][nc]):
                        visited.add(npos)
                        queue.append(npos)
        return None

    # ------------------------------------------------------------------
    # Door graph management
    # ------------------------------------------------------------------

    def _update_door_graph(self, state):
        """Scan visible tiles for doors, add to graph, connect same-room doors."""
        if not state.map or state.player_pos is None or state.player_world_pos is None:
            return
        pr, pc = state.player_pos
        wx, wy = state.player_world_pos

        visible_doors = []
        for lr in range(len(state.map)):
            for lc in range(len(state.map[lr])):
                ch = state.map[lr][lc]
                if ch in ("'", "+"):
                    drc = (wy + (lr - pr), wx + (lc - pc))
                    if 0 <= drc[0] < 100 and 0 <= drc[1] < 100:
                        visible_doors.append(drc)
                        if drc not in self.door_graph:
                            self.door_graph[drc] = set()

        player_rc = (wy, wx)
        player_ch = state.map[pr][pc] if 0 <= pr < len(state.map) and 0 <= pc < len(state.map[0]) else None
        if player_ch in ("'", "+"):
            if player_rc not in self.door_graph:
                self.door_graph[player_rc] = set()
            self.explored_doors.add(player_rc)
            if self.last_door_wpos and self.last_door_wpos != player_rc:
                self.door_graph[player_rc].add(self.last_door_wpos)
                self.door_graph.setdefault(self.last_door_wpos, set()).add(player_rc)
            self.last_door_wpos = player_rc

        if len(visible_doors) > 1:
            for i in range(len(visible_doors)):
                for j in range(i + 1, len(visible_doors)):
                    self.door_graph[visible_doors[i]].add(visible_doors[j])
                    self.door_graph[visible_doors[j]].add(visible_doors[i])

    # ------------------------------------------------------------------
    # Utility helpers
    # ------------------------------------------------------------------

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
            f"stuck={self.stuck_turns}"
        )

    @staticmethod
    def _goal_thought(state, prefix, goal_local):
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

    @staticmethod
    def _room_is_visible(grid, pos):
        """True when the room appears lit (floor tiles visible beyond 1 step)."""
        r, c = pos
        rows = len(grid)
        cols = len(grid[0]) if rows else 0
        far_floor = 0
        for dr in range(-3, 4):
            for dc in range(-3, 4):
                if abs(dr) <= 1 and abs(dc) <= 1:
                    continue
                nr, nc = r + dr, c + dc
                if 0 <= nr < rows and 0 <= nc < cols:
                    ch = grid[nr][nc]
                    if ch in (".", "'", "<", ">"):
                        far_floor += 1
        return far_floor >= 3

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
    def _monster_signal_reliable(state):
        """Treat very large monster counts as parser noise."""
        return len(state.monsters) <= 12

    # ------------------------------------------------------------------
    # Equipment / gear scoring
    # ------------------------------------------------------------------

    def _gear_slot_kind(self, item_name):
        n = item_name.lower()
        if any(k in n for k in ("torch", "lantern")):
            return "light"
        if "shield" in n:
            return "shield"
        if "boots" in n:
            return "boots"
        if any(k in n for k in ("helm", "cap")):
            return "head"
        if "cloak" in n:
            return "cloak"
        if any(k in n for k in ("gauntlets", "gloves")):
            return "hands"
        if any(k in n for k in ("armor", "mail", "robe")):
            return "body"
        if any(
            k in n
            for k in (
                "sword", "axe", "mace", "hammer", "morning star",
                "spear", "dagger", "whip", "club", "pickaxe",
                "shovel", "bow", "sling", "crossbow",
            )
        ):
            return "weapon"
        return None

    def _gear_score(self, item_name):
        """Score an item by learned observations, with a heuristic fallback."""
        n = item_name.lower()
        gear_strength = self.item_knowledge.get("gear_strength", {})
        learned = gear_strength.get(n, {}) if isinstance(gear_strength, dict) else {}

        if any(k in n for k in ("torch", "lantern")):
            return 12.0

        has_observations = isinstance(learned, dict) and (
            float(learned.get("observed_damage_avg", 0)) > 0
            or float(learned.get("observed_ac_best", 0)) > 0
        )

        score = 0.0
        if has_observations:
            dmg_avg = float(learned.get("observed_damage_avg", 0))
            to_dam = float(learned.get("observed_to_dam", 0))
            score += (dmg_avg + to_dam) * 1.5
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

    def _equipped_name_for_kind(self, equipment, kind):
        for _slot, name in equipment or []:
            if self._gear_slot_kind(name) == kind:
                return name
        return None

    def _is_gear_tested(self, item_name):
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

        Untested gear is always tried to learn its stats.
        Known gear replaces equipped only if strictly better by score.
        """
        kind = self._gear_slot_kind(item_name)
        if not kind:
            return self.wield_attempt_counts.get(item_name, 0) == 0

        if not self._is_gear_tested(item_name):
            return True

        equipped_name = self._equipped_name_for_kind(equipment, kind)
        if not equipped_name:
            return True

        return self._gear_score(item_name) > self._gear_score(equipped_name)

    # ------------------------------------------------------------------
    # Wield learning
    # ------------------------------------------------------------------

    def _learn_from_wield_feedback(self, state):
        message = state.last_message
        if not message:
            return

        lower = message.lower()

        m = re.search(r"you can't wield a\s+(.+?)!", lower)
        if m:
            item = m.group(1).strip()
            if item:
                self._learn_non_wieldable(item)
            return

        if "returns to your pack" in lower and self.last_equip_item_name:
            self.wield_attempt_counts[self.last_equip_item_name.lower()] = 99
            self.last_equip_item_name = None
            self.last_equip_baseline_ac = None
            return

        m = re.search(r"you are now wielding the\s+(.+?)\.?$", message, flags=re.IGNORECASE)
        if m:
            item = m.group(1).strip()
            if item:
                self._learn_successful_equip(
                    item, state.player_ac, state.damage_dice,
                    state.to_hit_bonus, state.to_dam_bonus,
                )
                kind = self._gear_slot_kind(item)
                if kind:
                    best = self.item_knowledge.get("best_by_slot", {}).get(kind)
                    if isinstance(best, dict) and best.get("name", "").lower() != item.lower():
                        self.pending_reequip_kind = kind

    @staticmethod
    def _dice_avg(dice_str):
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
            entry["observed_ac_avg"] = int(entry.get("observed_ac_total", 0)) / count
            entry["observed_ac_best"] = max(int(entry.get("observed_ac_best", 0)), delta)

        kind = self._gear_slot_kind(item_name)
        if kind == "weapon":
            if current_damage:
                entry["observed_damage"] = current_damage
                entry["observed_damage_avg"] = self._dice_avg(current_damage)
            if to_hit:
                entry["observed_to_hit"] = int(to_hit)
            if to_dam:
                entry["observed_to_dam"] = int(to_dam)
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
        """On pickup message, queue wield only if item is not known non-wieldable."""
        msg = (state.last_message or "").strip()
        if not msg:
            return
        m = re.search(r"you have an?\s+(.+?)\.?$", msg, flags=re.IGNORECASE)
        if not m:
            return
        picked = m.group(1).strip().lower()
        if not picked or self._is_known_non_wieldable(picked):
            return

        for slot, name in state.inventory or []:
            if name.lower() == picked:
                self.pending_pickup_equip_slot = slot
                return

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
            if self._is_known_non_wieldable(name):
                continue
            self.pending_pickup_equip_slot = slot
            return

    @staticmethod
    def _item_category(item_name):
        """Extract the general category from an item name.

        "Blue Potion" -> "potion", "Scroll labeled foo" -> "scroll".
        """
        n = item_name.strip().lower()
        if n.startswith("scroll labeled") or n.startswith("scroll"):
            return "scroll"
        parts = n.split()
        return parts[-1] if parts else n

    def _learn_non_wieldable(self, item_name):
        cat = self._item_category(item_name)
        if cat and cat not in self.learned_non_wieldable_categories:
            self.learned_non_wieldable_categories.add(cat)
            self.knowledge_dirty = True

    def _is_known_non_wieldable(self, item_name):
        return self._item_category(item_name) in self.learned_non_wieldable_categories

    # ------------------------------------------------------------------
    # Monster learning
    # ------------------------------------------------------------------

    def _learn_from_monster_feedback(self, message, hp_loss=0):
        if not message:
            return

        self.turns_since_combat_feedback += 1
        msg = message.strip()

        m = re.search(r"you hit the\s+(.+?)\.?$", msg, flags=re.IGNORECASE)
        if m:
            self.turns_since_combat_feedback = 0
            self._monster_note(m.group(1), "hits", 1)
            self._weapon_combat_note("hits")
            return

        m = re.search(r"you miss the\s+(.+?)\.?$", msg, flags=re.IGNORECASE)
        if m:
            self.turns_since_combat_feedback = 0
            self._monster_note(m.group(1), "misses", 1)
            self._weapon_combat_note("misses")
            return

        m = re.search(r"you have slain the\s+(.+?)\.?$", msg, flags=re.IGNORECASE)
        if m:
            self.turns_since_combat_feedback = 0
            self._monster_note(m.group(1), "kills", 1)
            self._weapon_combat_note("kills")
            return

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
                "hits": 0, "misses": 0, "hits_taken": 0, "kills": 0,
                "attacks": {}, "damage_taken_total": 0,
                "damage_instances": 0, "max_observed_hit": 0,
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
                "hits": 0, "misses": 0, "hits_taken": 0, "kills": 0,
                "attacks": {}, "damage_taken_total": 0,
                "damage_instances": 0, "max_observed_hit": 0,
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
                "hits": 0, "misses": 0, "hits_taken": 0, "kills": 0,
                "attacks": {}, "damage_taken_total": 0,
                "damage_instances": 0, "max_observed_hit": 0,
            },
        )
        entry["damage_taken_total"] = int(entry.get("damage_taken_total", 0)) + int(hp_loss)
        entry["damage_instances"] = int(entry.get("damage_instances", 0)) + 1
        entry["max_observed_hit"] = max(int(entry.get("max_observed_hit", 0)), int(hp_loss))
        self.knowledge_dirty = True

    def _weapon_combat_note(self, event):
        weapon = self.current_wielded_weapon
        if not weapon:
            return
        gear_strength = self.item_knowledge.setdefault("gear_strength", {})
        entry = gear_strength.setdefault(
            weapon.lower(),
            {
                "successes": 0,
                "observed_ac_total": 0, "observed_ac_count": 0,
                "observed_ac_avg": 0.0, "observed_ac_best": 0,
                "observed_damage": "", "observed_damage_avg": 0.0,
                "observed_to_hit": 0, "observed_to_dam": 0,
                "combat_hits": 0, "combat_misses": 0,
                "combat_kills": 0, "combat_turns": 0,
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
        hit_pressure = hits_taken / max(1.0, hits_taken + float(entry.get("misses", 0)))
        breath_bonus = 1.0 if (entry.get("attacks") or {}).get("breathes", 0) else 0.0
        return (avg_dmg * 1.4) + (max_hit * 0.6) + (hit_pressure * 2.0) + breath_bonus

    def _monster_confidence(self, monster_name):
        if not monster_name:
            return 0.0
        entry = self.monster_knowledge.get(monster_name.lower(), {})
        samples = float(entry.get("damage_instances", 0)) + float(entry.get("hits_taken", 0))
        return min(1.0, samples / 6.0)

    # ------------------------------------------------------------------
    # Scroll / consumable learning
    # ------------------------------------------------------------------

    def _learn_from_scroll_feedback(self, message):
        if not message:
            return

        msg = message.strip()
        m = re.search(r"you read the\s+scroll labeled\s+(.+?)\.?$", msg, flags=re.IGNORECASE)
        if m:
            label = m.group(1).strip().lower()
            if label:
                self.pending_scroll_label = label
            return

        if not self.pending_scroll_label:
            return

        lower = msg.lower()
        if any(k in lower for k in ("you read the", "you are now", "you miss", "you hit")):
            return

        if any(k in lower for k in ("choose an item", "which item")):
            self._learn_consumable_effect("scroll", {"prompts_choose": True}, lower)
            self.pending_scroll_label = None
            return

        self.pending_scroll_label = None

    @staticmethod
    def _take_state_snapshot(state):
        return {
            "hp": state.player_hp,
            "max_hp": state.player_max_hp,
            "ac": state.player_ac,
            "damage_dice": state.damage_dice,
            "to_hit": state.to_hit_bonus,
            "to_dam": state.to_dam_bonus,
            "world_pos": state.player_world_pos,
            "depth": state.dungeon_depth,
            "equipment": [(s, n) for s, n in (state.equipment or [])],
            "inventory": [(s, n) for s, n in (state.inventory or [])],
        }

    @staticmethod
    def _diff_state(before, after):
        changes = {}
        if before["hp"] != after["hp"]:
            changes["hp_delta"] = after["hp"] - before["hp"]
        if before["ac"] != after["ac"]:
            changes["ac_delta"] = after["ac"] - before["ac"]
        if before["damage_dice"] != after["damage_dice"]:
            changes["damage_changed"] = True
            changes["damage_before"] = before["damage_dice"]
            changes["damage_after"] = after["damage_dice"]
        if before["to_hit"] != after["to_hit"]:
            changes["to_hit_delta"] = after["to_hit"] - before["to_hit"]
        if before["to_dam"] != after["to_dam"]:
            changes["to_dam_delta"] = after["to_dam"] - before["to_dam"]
        if before["world_pos"] != after["world_pos"]:
            changes["position_changed"] = True
        if before["depth"] != after["depth"]:
            changes["depth_changed"] = True
        before_equip = set(n for _, n in before["equipment"])
        after_equip = set(n for _, n in after["equipment"])
        if before_equip != after_equip:
            changes["equipment_changed"] = True
        return changes

    def _learn_from_consumable_feedback(self, state):
        """Track effects after quaffing or reading by observing state changes."""
        messages = getattr(state, "messages", [])
        msg = (state.last_message or "").strip()
        if not msg and not messages:
            return

        lower = msg.lower()
        all_lower = " ".join(m.lower() for m in messages)

        m = re.search(r"you drank the\s+(.+?)\.?$", lower)
        if not m:
            for line in messages:
                m = re.search(r"you drank the\s+(.+?)\.?$", line.strip(), flags=re.IGNORECASE)
                if m:
                    break
        if m:
            flavor = m.group(1).strip().lower()
            if flavor:
                self.pending_consumable_flavor = flavor
                self.pre_use_snapshot = self._take_state_snapshot(state)
                self.use_cooldown = 2
            return

        m = re.search(r"you read the\s+(.+?)\.?$", lower)
        if not m:
            for line in messages:
                m = re.search(r"you read the\s+(.+?)\.?$", line.strip(), flags=re.IGNORECASE)
                if m:
                    break
        if m:
            flavor = m.group(1).strip().lower()
            if flavor:
                self.pending_consumable_flavor = flavor
                self.pre_use_snapshot = self._take_state_snapshot(state)
                self.use_cooldown = 2
            return

        if "you can't drink" in lower or "you can't read" in lower:
            self.pending_consumable_flavor = None
            self.pre_use_snapshot = None
            return

        if not self.pending_consumable_flavor:
            return

        if any(k in all_lower for k in ("choose an item", "which item")):
            cat = self._item_category(self.pending_consumable_flavor)
            self._learn_consumable_effect(cat, {"prompts_choose": True}, all_lower)
            return

        if any(k in lower for k in ("quaff which", "read which", "wield which")):
            return

        after_snapshot = self._take_state_snapshot(state)
        changes = self._diff_state(self.pre_use_snapshot, after_snapshot) if self.pre_use_snapshot else {}

        cat = self._item_category(self.pending_consumable_flavor)

        m = re.search(r"you have no more\s+(.+?)\s+of\s+(.+?)\.?$", all_lower)
        if m:
            true_identity = m.group(2).strip()
            if true_identity:
                self.flavor_map[self.pending_consumable_flavor] = true_identity
                self._learn_consumable_effect(cat, changes, all_lower, identity=true_identity)
            self.pending_consumable_flavor = None
            self.pre_use_snapshot = None
            return

        self._learn_consumable_effect(cat, changes, all_lower)

        if changes.get("hp_delta", 0) > 0:
            self.flavor_map[self.pending_consumable_flavor] = "healed"
        elif changes.get("hp_delta", 0) < 0:
            self.flavor_map[self.pending_consumable_flavor] = "harmed"
        elif changes.get("position_changed") or changes.get("depth_changed"):
            self.flavor_map[self.pending_consumable_flavor] = "teleport"
        elif changes.get("ac_delta"):
            self.flavor_map[self.pending_consumable_flavor] = "stat_change"
        elif not changes:
            self.flavor_map.setdefault(self.pending_consumable_flavor, "{tried}")

        self.pending_consumable_flavor = None
        self.pre_use_snapshot = None

    def _learn_consumable_effect(self, category, changes, raw_messages, identity=None):
        """Record an observed consumable effect type in persistent knowledge."""
        tags = []
        if changes.get("hp_delta", 0) > 0:
            tags.append("heals_hp")
        if changes.get("hp_delta", 0) < 0:
            tags.append("harms_hp")
        if changes.get("ac_delta", 0) != 0:
            tags.append("changes_ac")
        if changes.get("damage_changed"):
            tags.append("changes_damage")
        if changes.get("to_hit_delta", 0) != 0:
            tags.append("changes_to_hit")
        if changes.get("to_dam_delta", 0) != 0:
            tags.append("changes_to_dam")
        if changes.get("position_changed") or changes.get("depth_changed"):
            tags.append("changes_position")
        if changes.get("equipment_changed"):
            tags.append("changes_equipment")
        if changes.get("prompts_choose"):
            tags.append("prompts_choose")
        if not tags:
            tags.append("no_visible_effect")

        effect_key = (category or "unknown") + ":" + "+".join(sorted(tags))
        entry = self.consumable_knowledge.setdefault(effect_key, {"count": 0})
        entry["count"] = int(entry.get("count", 0)) + 1
        if identity:
            entry["identity"] = identity
        hp_d = changes.get("hp_delta", 0)
        if hp_d:
            entry["hp_delta_total"] = int(entry.get("hp_delta_total", 0)) + hp_d
        self.knowledge_dirty = True

    def _is_known_healing_flavor(self, item_name):
        return self.flavor_map.get(item_name.lower(), "") == "healed"

    def _is_known_bad_flavor(self, item_name):
        return self.flavor_map.get(item_name.lower(), "") == "harmed"

    def _find_inventory_consumable(self, inventory, category=None, exclude_bad=True):
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
        for slot, name in (inventory or []):
            if self._item_category(name) == "potion" and self._is_known_healing_flavor(name):
                return slot, name
        return None, None

    def _consider_consumable_use(self, state, adjacent):
        """Decide whether to use a consumable. Returns an action string or None."""
        if self.pending_use_slot or self.use_cooldown > 0:
            return None

        if state.player_max_hp > 0 and state.hp_pct < 0.50:
            slot, name = self._find_healing_potion(state.inventory)
            if slot:
                return self._initiate_use("q", slot, name)

        if adjacent is not None:
            return None

        if state.player_max_hp > 0 and state.hp_pct < 0.60:
            slot, name = self._find_inventory_consumable(state.inventory, category="potion")
            if slot and not self.flavor_map.get(name.lower()):
                return self._initiate_use("q", slot, name)

        if state.player_max_hp > 0 and state.hp_pct > 0.70:
            slot, name = self._find_inventory_consumable(state.inventory, category="scroll")
            if slot and not self.flavor_map.get(name.lower()):
                return self._initiate_use("r", slot, name)

        return None

    def _initiate_use(self, cmd, slot, item_name):
        self.pending_use_cmd = cmd
        self.pending_use_slot = slot
        self.pending_use_item_name = item_name
        self.await_use_prompt_turns = 3
        self.use_cooldown = 3
        self.mode = "use"
        cmd_name = {"q": "quaff", "r": "read"}.get(cmd, cmd)
        return self._record_decision(cmd, f"use_{cmd_name}_{item_name}")

    # ------------------------------------------------------------------
    # Persistence
    # ------------------------------------------------------------------

    def load_knowledge(self, file_path):
        try:
            with open(file_path, "r", encoding="utf-8") as f:
                data = json.load(f)
            cats = data.get("non_wieldable_categories", [])
            legacy_names = data.get("non_wieldable_names", [])
            monsters = data.get("monster_knowledge", {})
            item_k = data.get("item_knowledge", {})
            door_k = data.get("door_knowledge", {})
            map_k = data.get("map_knowledge", {})
            consumable_k = data.get("consumable_knowledge", {})
            self.learned_non_wieldable_categories.update(c.lower() for c in cats if isinstance(c, str))
            for n in legacy_names:
                if isinstance(n, str):
                    self.learned_non_wieldable_categories.add(self._item_category(n))
            # Migration: remove categories that are actually wieldable.
            for bad_cat in ("shield", "dagger"):
                self.learned_non_wieldable_categories.discard(bad_cat)
            if isinstance(monsters, dict):
                self.monster_knowledge = monsters
            if isinstance(item_k, dict):
                self.item_knowledge = item_k
                self.item_knowledge.setdefault("gear_strength", {})
                self.item_knowledge.setdefault("best_by_slot", {})
                # Migration: strip damage fields from non-weapon gear entries.
                for gname, gentry in self.item_knowledge.get("gear_strength", {}).items():
                    if self._gear_slot_kind(gname) != "weapon":
                        for dkey in ("observed_damage", "observed_damage_avg", "observed_to_hit", "observed_to_dam"):
                            gentry.pop(dkey, None)
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
            "schema_version": 5,
            "non_wieldable_categories": sorted(self.learned_non_wieldable_categories),
            "monster_knowledge": self.monster_knowledge,
            "consumable_knowledge": self.consumable_knowledge,
            "item_knowledge": self.item_knowledge,
            "door_knowledge": self.door_knowledge,
            "map_knowledge": self.map_knowledge,
        }
        with open(file_path, "w", encoding="utf-8") as f:
            json.dump(data, f, indent=2, sort_keys=True)
            f.write("\n")
        self.knowledge_dirty = False
