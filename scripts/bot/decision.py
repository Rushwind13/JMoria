"""decision.py - Priority decision engine for the JMoria bot (Phase 2)."""

from collections import deque
import json
import re

from . import pathfinding as pf

KEY_TO_DIR = {v: k for k, v in pf.DIR_TO_KEY.items()}


class DecisionEngine:
    """
    ============================================================
    DUNGEON PROGRESSION STRATEGY - ISSUE #205
    ============================================================
    
    GOAL HIERARCHY (from Issue #186: Goal-Stack Based Exploration):
      1. Explore all reachable floor tiles and interact with doors
      2. Eliminate visible monsters
      3. Collect and evaluate items/equipment
      4. DESCEND STAIRCASE to next dungeon level
    
    DOWNWARD PROGRESSION CRITERIA:
    -----------------------------------------------
    Before descending ('>'), ALL of these must be true:
    
    1. EXPLORATION COMPLETE
       - No unexplored walkable tiles remain on current level
       - All discovered doors have been interacted with
    
    2. HP RECOVERY (>= 70%)
       - Ensures player enters next level with safety margin
       - Low HP (< 50%) triggers rest in prior decision tier
       - Prevents descending into danger under-resourced
    
    3. COMBAT STABILITY
       - No visible adjacent monsters
       - No recent attacker (if combat_feedback is > N turns old)
       - Safe to transition without immediate threat
    
    4. WEAPON READINESS
       - At least basic melee weapon identified/wielded
       - Ensures ability to defend on next level
       - Unarmed descent is allowed but suboptimal
    
    SAFETY PROPERTIES:
    - If HP < 50%, rest takes priority (prior decision tier)
    - Exploration stuck detection prevents infinite loops
    - Phantoms (stuck combat without feedback) are skipped
    
    TELEMETRY:
    - prog_descend_ready:           All criteria met, proceeding
    - prog_descend_delay_low_hp:    Recovery in progress (70%+)
    - prog_descend_delay_exploring: Unexplored tiles remain
    - prog_descend_delay_combat:    Active threat or recent combat
    - prog_descend_delay_unready:   No weapon vs. combat readiness
    
    ============================================================
    """
    def __init__(self):
        self.visited_world = set()  # (depth, world_row, world_col)
        self.last_pos = None
        self.stuck_turns = 0
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
        self.pre_use_snapshot = None  # full state snapshot before consumable use
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
        self.progression_telemetry = {
            "depths_cleared": [],     # list of (depth, reason, turn_count)
            "last_descent_reason": None,
            "turns_at_depth": 0,
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
        # Goal-based exploration state machine.
        self.explore_phase = "head_east"       # head_east, seek_unvisited
        self.wall_follow_heading = None          # current heading during perimeter walk
        self.wall_follow_start_wpos = None       # world pos where perimeter walk began
        self.wall_follow_started = False         # True after first step from perimeter start
        self.door_momentum = False               # True after opening a door (push through next tick)
        self.known_map = [[' '] * 100 for _ in range(100)]  # persistent 100x100 dungeon map
        self._current_depth = 1
        self.failed_door_dirs_by_world = {}
        # Door graph: nodes = door world positions, edges = same-room connectivity.
        self.door_graph = {}        # wpos -> set of connected door wpos
        self.explored_doors = set() # doors the bot has stepped through
        self.last_door_wpos = None  # last door we stepped through (for linking)
        self.cached_path = []       # committed path (world coords) to follow
        self.cached_path_target = None  # tile wpos we're pathing toward
        # Tile-level exploration tracking.
        self.unexplored_tiles = set()  # walkable (row,col) seen but not yet cleared
        self.explored_tiles = set()    # walkable (row,col) we've cleared
        # Goal stack: LIFO list of (goal_type, (row,col)).
        # Types: "unexplored", "door", "item", "staircase"
        self.goal_stack = []
        self.pushed_goals = set()  # avoid duplicate pushes
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
            self.known_map = [[' '] * 100 for _ in range(100)]
            self.door_graph = {}
            self.explored_doors = set()
            self.last_door_wpos = None
            self.cached_path = []
            self.cached_path_target = None
            self.unexplored_tiles = set()
            self.explored_tiles = set()
            self.goal_stack = []
            self.pushed_goals = set()
            self.progression_telemetry["turns_at_depth"] = 0  # Reset turn counter for new depth
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

        # Escape lingering inventory/use prompts the bot didn't initiate.
        if "choose an item from inventory" in msg_lower:
            if not self.pending_wield_slot and not self.pending_use_slot:
                return self._record_decision("\x1b", "escape_stale_prompt")

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
            direction = self.last_action if self.last_action in "hjklyubn" else "h"
            self.last_open_dir = direction
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
                self._begin_room_explore(state.map, pos, motion_pos, heading="j")

        # 1) Survival first: rest when HP below 50% and no visible threats.
        if state.player_max_hp > 0 and state.hp_pct < 0.50:
            self.mode = "recover"
            adjacent = None
            if self._monster_signal_reliable(state):
                adjacent = self._adjacent_monster(pos, state.monsters)
            if adjacent is None and not state.monsters:
                return self._record_decision("R", "low_hp_rest_no_visible_threat")
            # Adjacent monster at low HP: must fight — fleeing is pointless
            # since monsters match player speed.  Fall through to combat.

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
            # Always bump-attack — monsters match player speed,
            # so fleeing just means getting hit while running.
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

        # === GOAL STACK: unified exploration + item collection ===
        self.mode = "seek"
        return self._pursue_goals(state, pos)

    # ------------------------------------------------------------------
    # Goal-based exploration state machine
    # ------------------------------------------------------------------

    def _explore(self, state, pos, motion_pos):
        """Goal-based exploration: pathfind to nearest unexplored tile."""
        grid = state.map

        # After stepping through a door, push one step forward then
        # switch to seek_unvisited on the following tick.
        if self.door_momentum:
            self.door_momentum = False
            heading = self.last_action if self.last_action in "hjklyubn" else "l"
            self.explore_phase = "seek_unvisited"
            if self._can_step(grid, pos, heading):
                return self._record_decision(heading, "door_push_thru")
            # Can't push forward — fall through to seek immediately.

        # head_east: initial walk east until blocked, then switch to seek.
        if self.explore_phase == "head_east":
            if self._can_step(grid, pos, "l"):
                return self._record_decision("l", "head_east")
            # Hit a wall or obstacle — switch to tile-based exploration.
            self.explore_phase = "seek_unvisited"

        # seek_unvisited: always pathfind to nearest unexplored tile.
        if self.explore_phase in ("seek_unvisited", "navigate_exit", "enter_room", "reveal_perimeter"):
            self.explore_phase = "seek_unvisited"
            wpos = state.player_world_pos
            if wpos:
                mpos = self._wpos_to_rc(wpos)  # map coords (row, col)
                # Follow cached path if still valid.
                if self.cached_path and self.cached_path_target is not None:
                    if self.cached_path_target not in self.unexplored_tiles:
                        # Target was explored (stepped on or lit-room auto-clear).
                        self.cached_path = []
                        self.cached_path_target = None
                    elif self.cached_path and self.cached_path[0] == mpos:
                        self.cached_path.pop(0)
                    elif mpos not in self.cached_path:
                        # Off-path (combat detour etc). Recompute.
                        self.cached_path = []
                        self.cached_path_target = None
                    else:
                        # Skip steps we've already passed.
                        while self.cached_path and self.cached_path[0] != mpos:
                            self.cached_path.pop(0)
                        if self.cached_path:
                            self.cached_path.pop(0)  # remove current pos

                # Follow cached path.
                if self.cached_path:
                    nxt = self.cached_path[0]
                    dkey = pf.DIR_TO_KEY.get((nxt[0] - mpos[0], nxt[1] - mpos[1]))
                    if dkey:
                        # Check if next step is a closed door — open it.
                        nxt_ch = self.known_map[nxt[0]][nxt[1]] if 0 <= nxt[0] < 100 and 0 <= nxt[1] < 100 else None
                        if nxt_ch == '+':
                            self.last_open_dir = dkey
                            self.pending_keys = [dkey]
                            self.door_momentum = True
                            self.cached_path = []
                            self.cached_path_target = None
                            return self._record_decision("o", f"path_open_{dkey}")
                        if self._can_step(grid, pos, dkey):
                            dist = len(self.cached_path)
                            nunex = len(self.unexplored_tiles)
                            return self._record_decision(dkey, f"follow_path_d{dist}_u{nunex}")
                    # Path step not walkable — recompute.
                    self.cached_path = []
                    self.cached_path_target = None

                # Find nearest unexplored tile.
                target = self._nearest_unexplored_tile(mpos)
                if target:
                    dist = pf.heuristic(mpos, target)
                    if dist <= 1:
                        dkey = pf.DIR_TO_KEY.get((target[0] - mpos[0], target[1] - mpos[1]))
                        if dkey:
                            # Closed door — open it.
                            tch = self.known_map[target[0]][target[1]]
                            if tch == '+':
                                self.last_open_dir = dkey
                                self.pending_keys = [dkey]
                                self.door_momentum = True
                                return self._record_decision("o", f"open_door_{dkey}")
                            if self._can_step(grid, pos, dkey):
                                nunex = len(self.unexplored_tiles)
                                return self._record_decision(dkey, f"step_unexplored_u{nunex}")
                        # Can't step — mark explored and let next tick retry.
                        self.unexplored_tiles.discard(target)
                        self.explored_tiles.add(target)
                    else:
                        # Pathfind to the target (or a walkable neighbor for doors).
                        path = pf.path_to(self.known_map, mpos, target)
                        if not path and self.known_map[target[0]][target[1]] == '+':
                            adj = self._walkable_neighbor_of(target)
                            if adj:
                                path = pf.path_to(self.known_map, mpos, adj)
                        if path and len(path) >= 2:
                            self.cached_path = path[1:]
                            self.cached_path_target = target
                            nxt = self.cached_path[0]
                            key = pf.DIR_TO_KEY.get((nxt[0] - mpos[0], nxt[1] - mpos[1]))
                            if key:
                                # First step might be a closed door.
                                nxt_ch = self.known_map[nxt[0]][nxt[1]]
                                if nxt_ch == '+':
                                    self.last_open_dir = key
                                    self.pending_keys = [key]
                                    self.door_momentum = True
                                    self.cached_path = []
                                    self.cached_path_target = None
                                    return self._record_decision("o", f"path_open_{key}")
                                nunex = len(self.unexplored_tiles)
                                return self._record_decision(key, f"seek_unexplored_d{dist}_u{nunex}")
                        # Pathfind failed — mark unreachable.
                        self.unexplored_tiles.discard(target)
                        self.explored_tiles.add(target)

        # Stuck fallback: try any walkable direction.
        for key in "ljkhyubn":
            if self._can_step(grid, pos, key):
                return self._record_decision(key, f"stuck_fallback_{key}")

        self.mode = "idle"
        return self._record_decision(".", "idle_wait")

    def _begin_room_explore(self, grid, pos, motion_pos, heading="j"):
        """Initialize exploration of a new room or corridor."""
        if self._is_hallway_tile(grid, pos):
            # Hallway: no perimeter needed, pathfind along it.
            self.explore_phase = "seek_unvisited"
        elif self._room_is_visible(grid, pos):
            # Lit room: all tiles already revealed, skip perimeter.
            self.explore_phase = "seek_unvisited"
        else:
            # Dark room: wall-follow perimeter to reveal edges.
            self.explore_phase = "reveal_perimeter"
            self.wall_follow_heading = heading
            self.wall_follow_start_wpos = motion_pos
            self.wall_follow_started = False

    @staticmethod
    def _room_is_visible(grid, pos):
        """Check if the room appears lit (floor tiles visible beyond 1 step)."""
        r, c = pos
        rows = len(grid)
        cols = len(grid[0]) if rows else 0
        far_floor = 0
        for dr in range(-3, 4):
            for dc in range(-3, 4):
                if abs(dr) <= 1 and abs(dc) <= 1:
                    continue  # skip adjacent tiles (always visible)
                nr, nc = r + dr, c + dc
                if 0 <= nr < rows and 0 <= nc < cols:
                    ch = grid[nr][nc]
                    if ch in (".", "'", "<", ">"):
                        far_floor += 1
        return far_floor >= 3

    # ------------------------------------------------------------------
    # Known-map management (persistent 100x100 dungeon grid)
    # ------------------------------------------------------------------

    @staticmethod
    def _wpos_to_rc(wpos):
        """Convert world pos (X, Y) to map coords (row, col) = (Y, X)."""
        return (wpos[1], wpos[0])

    @staticmethod
    def _rc_to_wpos(rc):
        """Convert map coords (row, col) to world pos (X, Y) = (col, row)."""
        return (rc[1], rc[0])

    def _update_known_map(self, state):
        """Blit visible screen tiles onto the persistent 100x100 known_map."""
        if not state.map or state.player_pos is None or state.player_world_pos is None:
            return
        pr, pc = state.player_pos          # screen (row, col)
        wx, wy = state.player_world_pos    # world (X=col, Y=row)
        monster_set = {(mr, mc) for mr, mc, _ in state.monsters}
        item_set = {(ir, ic) for ir, ic, _ in state.items}
        for lr in range(len(state.map)):
            for lc in range(len(state.map[lr])):
                ch = state.map[lr][lc]
                if ch == ' ':
                    continue  # out of field-of-view, no info
                gr = wy + (lr - pr)    # map row = world Y + screen row delta
                gc = wx + (lc - pc)    # map col = world X + screen col delta
                if 0 <= gr < 100 and 0 <= gc < 100:
                    if (lr, lc) in monster_set or (lr, lc) in item_set or ch == '@':
                        self.known_map[gr][gc] = '.'
                    else:
                        self.known_map[gr][gc] = ch

        # Stamp unknown 8-neighbors of player's map position as wall.
        # The player always sees adjacent tiles; anything still unknown is solid rock.
        prow, pcol = wy, wx   # player's map coords
        for dr in (-1, 0, 1):
            for dc in (-1, 0, 1):
                if dr == 0 and dc == 0:
                    continue
                nr, nc = prow + dr, pcol + dc
                if 0 <= nr < 100 and 0 <= nc < 100:
                    if self.known_map[nr][nc] == ' ':
                        self.known_map[nr][nc] = '#'

    def _update_exploration_sets(self, state):
        """Update unexplored/explored tile sets based on current visibility.

        Every visible walkable tile is added to unexplored (if new).
        Current tile is always cleared (moved to explored).
        In a lit room, floor tiles are auto-cleared; doors stay unexplored
        until the bot steps through them.
        """
        if not state.map or state.player_pos is None or state.player_world_pos is None:
            return
        pr, pc = state.player_pos          # screen (row, col)
        wx, wy = state.player_world_pos    # world (X=col, Y=row)
        player_rc = (wy, wx)               # map coords (row, col)
        monster_set = {(mr, mc) for mr, mc, _ in state.monsters}
        item_set = {(ir, ic) for ir, ic, _ in state.items}
        in_lit = self._room_is_visible(state.map, state.player_pos)

        for lr in range(len(state.map)):
            for lc in range(len(state.map[lr])):
                ch = state.map[lr][lc]
                if ch == ' ':
                    continue
                gr = wy + (lr - pr)    # map row
                gc = wx + (lc - pc)    # map col
                if not (0 <= gr < 100 and 0 <= gc < 100):
                    continue
                tw = (gr, gc)
                if tw in self.explored_tiles:
                    continue
                # Monster/item/player overlays are walkable floor.
                eff = ch
                if (lr, lc) in monster_set or (lr, lc) in item_set or ch == '@':
                    eff = '.'
                if eff not in ('.', "'", '+', '<', '>'):
                    continue  # wall / rock / non-walkable
                # Add to unexplored if we haven't seen it before.
                if tw not in self.unexplored_tiles:
                    self.unexplored_tiles.add(tw)
                # In lit rooms, auto-clear tiles not on the exploration
                # frontier.  The frontier check (_has_unknown_neighbor)
                # naturally keeps closed doors unexplored (their far side
                # is always unknown) and keeps open doors that lead into
                # dark areas.  Interior open doors get cleared so the bot
                # doesn't bounce between them.
                if in_lit and not self._has_unknown_neighbor(tw):
                    self.unexplored_tiles.discard(tw)
                    self.explored_tiles.add(tw)

        # Always clear current position.
        self.unexplored_tiles.discard(player_rc)
        self.explored_tiles.add(player_rc)

    # ------------------------------------------------------------------
    # Goal stack
    # ------------------------------------------------------------------

    def _update_goal_stack(self, state):
        """Push newly-visible doors and items onto the goal stack.

        Doors go on first (lower priority), items on top (higher priority).
        Duplicates are suppressed via pushed_goals.
        """
        if not state.map or state.player_pos is None or state.player_world_pos is None:
            return
        pr, pc = state.player_pos
        wx, wy = state.player_world_pos

        new_doors = []
        new_items = []

        # Scan visible doors that are still unexplored.
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

        # Scan visible items.
        for ir, ic, _ich in state.items:
            grc = (wy + (ir - pr), wx + (ic - pc))
            if not (0 <= grc[0] < 100 and 0 <= grc[1] < 100):
                continue
            key = ("item", grc)
            if key not in self.pushed_goals:
                self.pushed_goals.add(key)
                new_items.append(key)

        # Push doors (lower), then items (higher = processed first).
        self.goal_stack.extend(new_doors)
        self.goal_stack.extend(new_items)

    def _pursue_goals(self, state, pos):
        """Work through the goal stack: pathfind to the top goal."""
        grid = state.map
        wpos = state.player_world_pos
        if not wpos:
            return self._record_decision(".", "no_wpos")
        mpos = self._wpos_to_rc(wpos)
        pr, pc = state.player_pos
        wx, wy = wpos

        # After stepping through a door, push one step forward.
        if self.door_momentum:
            self.door_momentum = False
            heading = self.last_action if self.last_action in "hjklyubn" else "l"
            if self._can_step(grid, pos, heading):
                return self._record_decision(heading, "door_push_thru")

        # head_east: initial walk east until blocked.
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
                # Item reached (we're standing on it) -> pop.
                if mpos == grc:
                    self.goal_stack.pop()
                    continue
                # Item might have been picked up or disappeared.
                # We can't cheaply verify, so trust and pathfind.
                break
            elif gtype == "door":
                # Door explored (stepped through or auto-cleared) -> pop.
                if grc in self.explored_tiles:
                    self.goal_stack.pop()
                    continue
                if mpos == grc:
                    self.goal_stack.pop()
                    continue
                break
            elif gtype == "unexplored":
                if grc not in self.unexplored_tiles:
                    self.goal_stack.pop()
                    continue
                if mpos == grc:
                    self.goal_stack.pop()
                    # Commit to current direction: push next adjacent
                    # unexplored tile, preferring the direction of travel.
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
                    # At staircase - check if safe to descend (Issue #205)
                    can_descend, reason = self._can_safely_descend_stairs(state)
                    if can_descend:
                        self.goal_stack.pop()
                        # Record progression telemetry
                        self.progression_telemetry["last_descent_reason"] = reason
                        self.progression_telemetry["depths_cleared"].append(
                            (self._current_depth, reason, len(self.visited_world))
                        )
                        return self._record_decision(">", reason)
                    else:
                        # Not ready to descend - pop staircase goal and seek other goals
                        # to continue exploration/recovery
                        self.goal_stack.pop()
                        self.last_thought = reason
                        # Will fall through to search for other goals
                break
            else:
                self.goal_stack.pop()

        # If stack empty, push nearest unexplored tile or staircase.
        if not self.goal_stack:
            target = self._nearest_unexplored_tile(mpos)
            if target:
                self.goal_stack.append(("unexplored", target))
            else:
                stair = self._find_staircase(mpos)
                if stair:
                    self.goal_stack.append(("staircase", stair))

        if not self.goal_stack:
            # Nothing to do — stuck fallback.
            for key in "ljkhyubn":
                if self._can_step(grid, pos, key):
                    return self._record_decision(key, f"stuck_fallback_{key}")
            return self._record_decision(".", "idle_wait")

        # Pathfind to top goal.
        gtype, grc = self.goal_stack[-1]

        # Opportunistic: if an item goal is adjacent, step on it now
        # rather than pathing around it to reach a further goal.
        if gtype != "item":
            for i in range(len(self.goal_stack) - 1, -1, -1):
                if self.goal_stack[i][0] == "item":
                    irc = self.goal_stack[i][1]
                    if pf.heuristic(mpos, irc) == 1:
                        # Promote: move item goal to top of stack.
                        self.goal_stack.append(self.goal_stack.pop(i))
                        gtype, grc = self.goal_stack[-1]
                        self.cached_path = []
                        self.cached_path_target = None
                        break

        # Invalidate cached path if target changed.
        if self.cached_path_target != grc:
            self.cached_path = []
            self.cached_path_target = None

        # Maintain cached path.
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
                if nxt_ch == '+':
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

        # Compute new path.
        dist = pf.heuristic(mpos, grc)
        if dist <= 1:
            dkey = pf.DIR_TO_KEY.get((grc[0] - mpos[0], grc[1] - mpos[1]))
            if dkey:
                tch = self.known_map[grc[0]][grc[1]]
                if tch == '+':
                    self.last_open_dir = dkey
                    self.pending_keys = [dkey]
                    self.door_momentum = True
                    return self._record_decision("o", f"goal_open_{dkey}")
                if self._can_step(grid, pos, dkey):
                    return self._record_decision(dkey, f"goal_{gtype}_step")
            # Can't step to this target — pop it and retry.
            self.goal_stack.pop()
            return self._pursue_goals(state, pos)

        path_target = grc
        path = pf.path_to(self.known_map, mpos, path_target)
        if not path and self.known_map[grc[0]][grc[1]] == '+':
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
                if nxt_ch == '+':
                    self.last_open_dir = dkey
                    self.pending_keys = [dkey]
                    self.door_momentum = True
                    self.cached_path = []
                    self.cached_path_target = None
                    return self._record_decision("o", f"goal_open_{dkey}")
                if self._can_step(grid, pos, dkey):
                    return self._record_decision(dkey, f"goal_{gtype}_seek_d{dist}")

        # Pathfind failed — pop this goal and try next.
        self.goal_stack.pop()
        if self.goal_stack:
            self.cached_path = []
            self.cached_path_target = None
            return self._pursue_goals(state, pos)

        # Truly stuck.
        for key in "ljkhyubn":
            if self._can_step(grid, pos, key):
                return self._record_decision(key, f"stuck_fallback_{key}")
        return self._record_decision(".", "idle_wait")

    def _find_staircase(self, mpos):
        """Find nearest '>' in known_map via BFS."""
        return pf.find_nearest_target(
            self.known_map, mpos,
            lambda ch, p: ch == '>',
        )

    def _can_safely_descend_stairs(self, state):
        """
        Check if bot meets all progression criteria before descending.
        
        Returns: (can_descend: bool, reason: str)
        
        Criteria (Issue #205 - Downward Progression Strategy):
        1. All unexplored tiles explored
        2. HP >= 70% (recovery threshold)
        3. No visible adjacent monsters
        4. Min weapon readiness (unarmed OK but suboptimal)
        
        Telemetry reasons:
        - prog_descend_ready
        - prog_descend_delay_exploring
        - prog_descend_delay_low_hp
        - prog_descend_delay_combat
        - prog_descend_delay_unready
        """
        
        # 1. Exploration Complete: no unexplored tiles remain
        if self.unexplored_tiles:
            return (False, "prog_descend_delay_exploring")
        
        # 2. HP Recovery: must be >= 70%
        if state.player_max_hp > 0:
            hp_pct = state.player_hp / state.player_max_hp
            if hp_pct < 0.70:
                return (False, f"prog_descend_delay_low_hp_{hp_pct:.0%}")
        
        # 3. Combat Stability: no adjacent monsters
        if self._monster_signal_reliable(state):
            pos = state.player_pos
            adjacent = self._adjacent_monster(pos, state.monsters)
            if adjacent:
                return (False, "prog_descend_delay_combat_adjacent")
        
        # 4. Recent combat check: ensure sufficient cooldown
        # If we were just attacked, wait a few turns before descending
        if self.recent_attacker_name and self.turns_since_combat_feedback < 3:
            return (False, "prog_descend_delay_combat_recent")
        
        # All criteria met
        return (True, "prog_descend_ready")

    def _find_frontier_target(self, world_pos):
        """Nearest walkable tile in known_map adjacent to unknown territory."""
        return pf.find_nearest_target(
            self.known_map, world_pos,
            lambda ch, p: pf.is_walkable(ch) and self._has_unknown_neighbor(p),
        )

    def _has_unknown_neighbor(self, world_pos):
        """True if any 8-neighbor in known_map is unknown (' ')."""
        r, c = world_pos
        for dr in (-1, 0, 1):
            for dc in (-1, 0, 1):
                if dr == 0 and dc == 0:
                    continue
                nr, nc = r + dr, c + dc
                if 0 <= nr < 100 and 0 <= nc < 100:
                    if self.known_map[nr][nc] == ' ':
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
        """BFS on known_map to find nearest unexplored tile by walk distance.

        Walks through walkable tiles. At each tile, checks if it or any
        8-neighbor is in unexplored_tiles (handles closed doors which are
        SOLID and thus not directly walkable by BFS).
        """
        if not self.unexplored_tiles:
            return None
        visited = {wpos}
        queue = deque([wpos])
        while queue:
            cur = queue.popleft()
            # If current tile is unexplored and walkable, return it.
            if cur in self.unexplored_tiles and pf.is_walkable(self.known_map[cur[0]][cur[1]]):
                return cur
            # Check 8-neighbors for unexplored closed doors (solid, not BFS-reachable).
            for dr, dc in pf.DIRS_8:
                nr, nc = cur[0] + dr, cur[1] + dc
                if 0 <= nr < 100 and 0 <= nc < 100:
                    npos = (nr, nc)
                    if npos in self.unexplored_tiles and self.known_map[nr][nc] == '+':
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
        pr, pc = state.player_pos          # screen (row, col)
        wx, wy = state.player_world_pos    # world (X=col, Y=row)

        # Collect all doors visible on screen (as map coords row, col).
        visible_doors = []
        for lr in range(len(state.map)):
            for lc in range(len(state.map[lr])):
                ch = state.map[lr][lc]
                if ch in ("'", "+"):
                    drc = (wy + (lr - pr), wx + (lc - pc))  # map (row, col)
                    if 0 <= drc[0] < 100 and 0 <= drc[1] < 100:
                        visible_doors.append(drc)
                        if drc not in self.door_graph:
                            self.door_graph[drc] = set()

        # If we're standing on a door, mark it explored.
        player_rc = (wy, wx)  # map (row, col)
        player_ch = state.map[pr][pc] if 0 <= pr < len(state.map) and 0 <= pc < len(state.map[0]) else None
        if player_ch in ("'", "+"):
            if player_rc not in self.door_graph:
                self.door_graph[player_rc] = set()
            self.explored_doors.add(player_rc)
            # Link to the last door we came from (hallway connection).
            if self.last_door_wpos and self.last_door_wpos != player_rc:
                self.door_graph[player_rc].add(self.last_door_wpos)
                self.door_graph.setdefault(self.last_door_wpos, set()).add(player_rc)
            self.last_door_wpos = player_rc

        # Connect all visible doors to each other (same-room edges).
        if len(visible_doors) > 1:
            for i in range(len(visible_doors)):
                for j in range(i + 1, len(visible_doors)):
                    self.door_graph[visible_doors[i]].add(visible_doors[j])
                    self.door_graph[visible_doors[j]].add(visible_doors[i])

    def _nearest_unexplored_door(self, wpos, skip_set=None):
        """BFS on known_map to find nearest unexplored door by walk distance."""
        exclude = (self.explored_doors | skip_set) if skip_set else self.explored_doors
        best = None
        best_dist = 1_000_000
        for dpos in self.door_graph:
            if dpos in exclude:
                continue
            path = pf.path_to(self.known_map, wpos, dpos)
            if path:
                d = len(path) - 1
                if d < best_dist:
                    best_dist = d
                    best = dpos
        door_tile = pf.find_nearest_target(
            self.known_map, wpos,
            lambda ch, p: ch in ("'", "+") and p not in exclude,
        )
        if door_tile:
            path = pf.path_to(self.known_map, wpos, door_tile)
            if path and len(path) - 1 < best_dist:
                best = door_tile
        return best

    def _nearest_unexplored_door_via_graph(self, wpos, skip_set=None):
        """BFS on the door graph to find nearest unexplored door by graph hops.

        Returns the first door on the path that we should walk toward.
        Falls back to _nearest_unexplored_door if graph BFS fails.
        """
        exclude = (self.explored_doors | skip_set) if skip_set else self.explored_doors
        # If we're on or adjacent to a graph door, start BFS from there.
        start_doors = []
        for dpos in self.door_graph:
            if pf.heuristic(wpos, dpos) <= 1:
                start_doors.append(dpos)
        if not start_doors:
            return self._nearest_unexplored_door(wpos, skip_set)

        # BFS on graph edges.
        visited = set()
        queue = deque()
        for sd in start_doors:
            queue.append(sd)
            visited.add(sd)
        while queue:
            cur = queue.popleft()
            if cur not in exclude:
                return cur
            for neighbor in self.door_graph.get(cur, ()):
                if neighbor not in visited:
                    visited.add(neighbor)
                    queue.append(neighbor)

        # Graph BFS found nothing — fall back to spatial search.
        return self._nearest_unexplored_door(wpos, skip_set)

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
        
        # Track turns at current depth for telemetry
        self.progression_telemetry["turns_at_depth"] += 1

    def get_progression_telemetry(self):
        """Export progression telemetry as a readable dict. (Issue #205 telemetry)."""
        return {
            "depths_descended": len(self.progression_telemetry["depths_cleared"]),
            "depths_cleared_list": [
                {
                    "depth": d[0],
                    "reason": d[1],
                    "tiles_visited": d[2]
                }
                for d in self.progression_telemetry["depths_cleared"]
            ],
            "last_descent_reason": self.progression_telemetry["last_descent_reason"],
            "turns_at_current_depth": self.progression_telemetry["turns_at_depth"],
        }

    @staticmethod
    def _map_signature(state):
        # Lightweight view signature to detect screen changes when world pos is unavailable.
        if not state.map:
            return ""
        rows = ["".join(r) for r in state.map]
        return "|".join(rows)

    def _is_visited_local(self, depth, local_pos):
        """Check if a local (screen) position has been visited, via world coords."""
        wpos = self._local_to_world_pos(local_pos)
        if wpos is None:
            return False  # can't tell — assume unvisited
        return (depth, wpos[0], wpos[1]) in self.visited_world

    def _local_to_world_pos(self, local_pos):
        """Convert local screen pos to world pos (X, Y) using current offsets."""
        mp = self.current_motion_pos  # (X, Y)
        pp = self.current_pos         # screen (row, col)
        if mp is None or pp is None:
            return None
        lr, lc = local_pos
        pr, pc = pp
        wx, wy = mp
        return (wx + (lc - pc), wy + (lr - pr))  # (X', Y')

    def _world_to_local_pos(self, world_pos):
        """Convert world pos (X, Y) to local screen pos using current offsets."""
        mp = self.current_motion_pos  # (X, Y)
        pp = self.current_pos         # screen (row, col)
        if mp is None or pp is None:
            return None
        wx, wy = mp
        pr, pc = pp
        x, y = world_pos  # (X, Y)
        return (pr + (y - wy), pc + (x - wx))  # screen (row, col)

    # Clockwise direction ordering for 8-way movement.
    _CW = ['k', 'u', 'l', 'n', 'j', 'b', 'h', 'y']
    _CW_IDX = {k: i for i, k in enumerate(_CW)}

    def _wall_follow_cw(self, grid, pos):
        """Clockwise wall-follow (left-hand rule, 8 directions).

        Used for dark room perimeter scanning only.  Wall stays on the LEFT.
        Skips doors so the bot stays inside the current room.
        """
        heading = self.wall_follow_heading
        if heading is None or heading not in self._CW_IDX:
            heading = 'j'
        h_idx = self._CW_IDX[heading]

        for offset in (-2, -1, 0, 1, 2, 3, -3, 4):
            cand = self._CW[((h_idx + offset) % 8)]
            if not self._can_step(grid, pos, cand):
                continue
            target = self._step_pos(pos, cand)
            if not target or not self._adjacent_to_wall(grid, target):
                continue
            ch = grid[target[0]][target[1]]
            if ch in ("'", "+"):
                continue  # skip doors to stay in room
            self.wall_follow_heading = cand
            return cand

        return None

    def _find_door_exit_step(self, grid, pos):
        """Find the first step toward the nearest door leading to unknown territory.

        Only returns doors that have at least one unknown neighbour in
        known_map, so the bot won't oscillate between fully-explored doors.
        """
        rows = len(grid)
        cols = len(grid[0]) if rows else 0
        if not rows or not cols:
            return None

        doors = []
        for r in range(rows):
            for c in range(cols):
                ch = grid[r][c]
                if ch in ("'", "+"):
                    # Only consider doors adjacent to unknown territory.
                    wpos = self._local_to_world_pos((r, c))
                    if wpos and self._has_unknown_neighbor(self._wpos_to_rc(wpos)):
                        doors.append((r, c, ch))

        if not doors:
            return None

        # Sort: unvisited doors first, then by distance.
        depth = getattr(self, '_current_depth', 1)
        doors.sort(key=lambda d: (
            1 if self._is_visited_local(depth, (d[0], d[1])) else 0,
            pf.heuristic(pos, (d[0], d[1]))
        ))

        for dr, dc, ch in doors:
            if (dr, dc) == pos:
                continue  # already on this tile
            if ch == "'":
                # Open door — pathfind directly to it.
                path = pf.path_to(grid, pos, (dr, dc))
                key = pf.first_step_key(path)
                if key:
                    return key
            else:
                # Closed door — pathfind to a walkable tile adjacent to it.
                # If we're already adjacent, return the direction to the door.
                dist = pf.heuristic(pos, (dr, dc))
                if dist == 1:
                    key = pf.DIR_TO_KEY.get((dr - pos[0], dc - pos[1]))
                    if key:
                        return key
                for ddr, ddc in pf.DIRS_8:
                    nr, nc = dr + ddr, dc + ddc
                    if not pf.in_bounds(grid, (nr, nc)):
                        continue
                    if not pf.is_walkable(grid[nr][nc]):
                        continue
                    path = pf.path_to(grid, pos, (nr, nc))
                    key = pf.first_step_key(path)
                    if key:
                        return key

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

    def _nearest_non_phantom_monster(self, state, pos):
        """Return screen (row, col) of nearest visible non-phantom monster."""
        pr, pc = pos
        best = None
        best_dist = 999
        for mr, mc, _ in state.monsters:
            if (mr, mc) == (pr, pc):
                continue
            wp = self._local_to_world(state, (mr, mc))
            if wp and wp in self.phantom_positions:
                continue
            d = pf.heuristic(pos, (mr, mc))
            if d < best_dist:
                best_dist = d
                best = (mr, mc)
        return best

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

    # Screen chars that represent equippable gear vs consumables.
    _GEAR_CHARS = frozenset('|/\\)[](')   # weapons and armor
    _CONSUMABLE_CHARS = frozenset('!?-_')  # potions, scrolls, wands, staves

    def _needs_gear(self, state):
        """True if missing a weapon or body armor — worth detouring for loot."""
        has_weapon = self.current_wielded_weapon is not None
        has_body_armor = False
        for _slot, name in state.equipment or []:
            if self._gear_slot_kind(name) == "body":
                has_body_armor = True
                break
        return not has_weapon or not has_body_armor

    def _step_toward_useful_item(self, state, pos):
        """Pathfind toward nearest useful ground item (gear or consumable)."""
        candidates = []
        for ir, ic, ich in state.items:
            if ich in self._GEAR_CHARS or ich in self._CONSUMABLE_CHARS:
                dist = pf.heuristic(pos, (ir, ic))
                candidates.append((dist, ir, ic, ich))
        candidates.sort()
        for _d, ir, ic, ich in candidates:
            path = pf.path_to(state.map, pos, (ir, ic))
            key = pf.first_step_key(path)
            if key:
                return self._record_decision(key, f"combat_seek_{ich}")
        return None

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

        # Wield attempt bounced back to inventory — transient failure
        # (e.g. cursed item blocking removal), NOT proof the type is unwieldable.
        if "returns to your pack" in lower and self.last_equip_item_name:
            item = self.last_equip_item_name.lower()
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

        # Only record damage/to-hit/to-dam for weapons — the stats sidebar
        # shows the *current weapon's* damage, not the item being equipped.
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
                "you read the",
                "you are now",
                "you miss",
                "you hit",
            )
        ):
            return

        # "choose an item" or "which item" is itself a meaningful effect: this
        # scroll prompts for a target (identify, enchant, etc.).
        if any(k in lower for k in ("choose an item", "which item")):
            self._learn_consumable_effect("scroll", {"prompts_choose": True}, lower)
            self.pending_scroll_label = None
            return

        # Capture first meaningful post-read effect text as learned effect note.
        self.pending_scroll_label = None

    # -- Consumable observation system --
    # Learns from screen messages and full state diffs, not from source code.

    @staticmethod
    def _take_state_snapshot(state):
        """Capture a snapshot of all observable game state for before/after diffing."""
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
        """Compare two state snapshots. Returns a dict of observed changes."""
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
        # Equipment changes (something appeared/disappeared from equip list).
        before_equip = set(n for _, n in before["equipment"])
        after_equip = set(n for _, n in after["equipment"])
        if before_equip != after_equip:
            changes["equipment_changed"] = True
        return changes

    def _learn_from_consumable_feedback(self, state):
        """Track effects after quaffing or reading by observing full state changes and messages."""
        messages = getattr(state, "messages", [])
        msg = (state.last_message or "").strip()
        if not msg and not messages:
            return

        lower = msg.lower()
        all_lower = " ".join(m.lower() for m in messages)

        # Detect quaff event: "You drank the <flavor name>."
        m = re.search(r"you drank the\s+(.+?)\.?$", lower)
        if not m:
            # Check all message lines in case the event scrolled.
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

        # Detect read event: "You read the <name>."
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

        # Detect failure messages and clear state.
        if "you can't drink" in lower or "you can't read" in lower:
            self.pending_consumable_flavor = None
            self.pre_use_snapshot = None
            return

        if not self.pending_consumable_flavor:
            return

        # "choose an item" or "which item" — this consumable prompts for a target.
        if any(k in all_lower for k in ("choose an item", "which item")):
            cat = self._item_category(self.pending_consumable_flavor)
            self._learn_consumable_effect(cat, {"prompts_choose": True}, all_lower)
            # Don't clear pending — the effect message may come on the next turn.
            return

        # Skip prompts that aren't effect messages.
        if any(k in lower for k in ("quaff which", "read which", "wield which")):
            return

        # Compute full state diff.
        after_snapshot = self._take_state_snapshot(state)
        changes = {}
        if self.pre_use_snapshot:
            changes = self._diff_state(self.pre_use_snapshot, after_snapshot)

        # Classify the consumable type.
        cat = self._item_category(self.pending_consumable_flavor)

        # Identity reveal: "You have no more Orange Potions of Cure Light Wounds"
        m = re.search(r"you have no more\s+(.+?)\s+of\s+(.+?)\.?$", all_lower)
        if m:
            true_identity = m.group(2).strip()
            if true_identity:
                self.flavor_map[self.pending_consumable_flavor] = true_identity
                self._learn_consumable_effect(cat, changes, all_lower, identity=true_identity)
            self.pending_consumable_flavor = None
            self.pre_use_snapshot = None
            return

        # Classify by observed state changes.
        self._learn_consumable_effect(cat, changes, all_lower)

        # Update per-run flavor map with effect summary.
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
        """Record an observed consumable effect type in persistent knowledge.

        Effects are keyed by a signature describing *what happened*, not the
        per-run flavor name.  E.g. 'potion:heals_hp', 'scroll:prompts_choose'.
        """
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

        entry = self.consumable_knowledge.setdefault(effect_key, {
            "count": 0,
        })
        entry["count"] = int(entry.get("count", 0)) + 1

        # Store identity if revealed (e.g. "minor healing").
        if identity:
            entry["identity"] = identity

        # Store representative stat deltas for reference.
        hp_d = changes.get("hp_delta", 0)
        if hp_d:
            entry["hp_delta_total"] = int(entry.get("hp_delta_total", 0)) + hp_d

        self.knowledge_dirty = True

    def _is_known_healing_flavor(self, item_name):
        """Check if we've observed this flavor healing us in the current run."""
        effect = self.flavor_map.get(item_name.lower(), "")
        return effect == "healed"

    def _is_known_bad_flavor(self, item_name):
        """Check if we've observed this flavor harming us in the current run."""
        effect = self.flavor_map.get(item_name.lower(), "")
        return effect == "harmed"

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
            item_k = data.get("item_knowledge", {})
            door_k = data.get("door_knowledge", {})
            map_k = data.get("map_knowledge", {})
            consumable_k = data.get("consumable_knowledge", {})
            self.learned_non_wieldable_categories.update(c.lower() for c in cats if isinstance(c, str))
            for n in legacy_names:
                if isinstance(n, str):
                    self.learned_non_wieldable_categories.add(self._item_category(n))
            # Migration: remove categories that are actually wieldable equipment.
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
            # Legacy: scroll_knowledge is no longer persisted separately;
            # scroll effects are recorded in consumable_knowledge.
            self.scroll_knowledge = {}
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
