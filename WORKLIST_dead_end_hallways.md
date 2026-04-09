Focusing on #181 (dead-end hallways) and #196 (hallway sidling).

## Build & Run Commands
- Build: `make clean && make ascii`
- Run: `tmux new-session -d -s crawler -x 200 -y 50 './jmoria 2>/tmp/jmoria_map.txt'`
- Capture: `tmux capture-pane -t crawler -p`
- Kill: `tmux kill-session -t crawler`

Files are in the repo root, not src. Let me read the key dungeon generation code.

Now I have full context. Here's the analysis and prioritized work items:

Situation Summary
Three related open issues, two in scope:

Issue	Title	In Scope?
#181	Reduce dead-end hallways	Yes
#196	Hallway sidling / double-wide corridors	Yes — same code area
#214	Isolated hallway (secret doors)	No — gated on Search (#117)
Why Dead-End Hallways Happen
Looking at DungeonMap.cpp:374-415, the CreateOneStep() flow:

Room → Hallways: A room tries 2–4 hallways from random walls. MakeHallStep() gets 20 retries each — if all fail CheckArea(), that direction simply gets nothing.

Hallway → Room (80%): A hallway tries to place a room at its end via MakeRoomStep() with 20 retries. If all 20 fail, the hallway is a dead end. No fallback — no truncation, no alternate direction, no pruning. The hallway just sits there ending in rock.

Hallway → Branch (20%): Hallway branches more hallways, which have the same dead-end risk recursively.

Depth limit: MAX_RECURDEPTH=10 causes all placement to return NULL at deep nesting, guaranteeing dead ends at the tree leaves.

There is no TruncateHallway or ConnectAdjacentStructures in the current codebase — the issue description and comment referenced those as desired features, not existing ones.

Prioritized Work Items
P0 — Highest Impact (fix the core problem)
1. Hallway-to-room fallback directions — When MakeRoomStep() fails in the hallway's primary direction, try the two perpendicular directions before giving up. This is the #1 producer of dead ends.

2. TruncateHallway — When a hallway's CheckArea() fails because it hits existing geometry, scan along the hallway direction to find the collision point and shorten the hallway to connect via a door. Converts failures into connections.

P1 — Medium Impact (connectivity & sidling)
3. ConnectAdjacentStructures (#196) — After carving any structure (room or hallway), scan its border for the floor|wall|wall|floor "sidling" pattern and place a connecting door. This fixes double-wide hallways AND adds connectivity. Can be done for both rooms and hallways in FillArea().

4. Dead-end pruning pass — After all generation is done, scan the map for hallway tiles with only one open neighbor (dead ends). "Erase" them back to wall, walking back until hitting a junction or door.

P2 — Lower Impact (polish)
5. Anti-sidle check for hallway placement — In CheckBorder(), reject a hallway placement if the border scan finds FLOOR tiles (not just WALL/DOOR). This prevents parallel hallways from ever being placed.

6. Force terminal rooms — When a hallway is at MAX_RECURDEPTH and would dead-end, try to place a small (2×2 or 3×3) room instead of a normal-sized one, with relaxed sizing.