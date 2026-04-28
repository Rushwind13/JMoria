#!/usr/bin/env python3
"""
crawler.py — JMoria dungeon crawler bot.

Launches jmoria in a tmux session using the ASCII renderer, drives through
the intro screens, then loops: read screen → decide → send command.

Usage:
    python3 scripts/crawler.py [--session NAME] [--verbose]
"""

import argparse
import sys
import time
import subprocess
import os
import shlex
from pathlib import Path

# Allow running from repo root: python3 scripts/crawler.py
sys.path.insert(0, os.path.dirname(__file__))

import bot.cmd as cmd
import bot.screen as screen
from bot.decision import DecisionEngine
from bot.state import GameState

# Seconds to wait for the game to draw a frame after a command
TICK_DELAY = 0.15

# Seconds to wait when polling for a screen transition
POLL_DELAY = 0.25
POLL_TIMEOUT = 10.0


def log(msg: str) -> None:
    print(msg, flush=True)


# ---------------------------------------------------------------------------
# Think pane (bot thought bubble, #207)
# ---------------------------------------------------------------------------

_think_pane_id = None
_snap_pane_id = None
_think_file = "/tmp/jmoria_think.txt"
_snap_file = "/tmp/jmoria_snap.txt"


def _create_think_pane(session: str) -> bool:
    """Create a 5-row bottom strip split left (think) / right (snapshot)."""
    global _think_pane_id, _snap_pane_id
    # Seed files.
    with open(_think_file, "w") as f:
        f.write("Think: Starting up...\nGoals: (none)\n")
    with open(_snap_file, "w") as f:
        f.write("(awaiting first snapshot)\n")
    # Left pane: per-turn think status.
    result = subprocess.run(
        ["tmux", "split-window", "-t", f"{session}:0.0", "-v", "-l", "5",
         "-d", "-P", "-F", "#{pane_id}",
         "sh", "-c", f"while true; do clear; cat {_think_file}; sleep 0.3; done"],
        capture_output=True, text=True,
    )
    if result.returncode != 0:
        log(f"[crawler] Failed to create think pane: {result.stderr.strip()}")
        return False
    _think_pane_id = result.stdout.strip()
    log(f"[crawler] Think pane created: {_think_pane_id}")
    # Right pane: periodic snapshot (split the think pane horizontally).
    result2 = subprocess.run(
        ["tmux", "split-window", "-t", _think_pane_id, "-h", "-l", "60",
         "-d", "-P", "-F", "#{pane_id}",
         "sh", "-c", f"while true; do clear; cat {_snap_file}; sleep 0.5; done"],
        capture_output=True, text=True,
    )
    if result2.returncode == 0:
        _snap_pane_id = result2.stdout.strip()
        log(f"[crawler] Snap pane created: {_snap_pane_id}")
    else:
        log(f"[crawler] Snap pane failed: {result2.stderr.strip()}")
    return True


def _destroy_think_pane() -> None:
    """Kill both think and snap panes if they exist."""
    global _think_pane_id, _snap_pane_id
    for label, pane_id in [("Snap", _snap_pane_id), ("Think", _think_pane_id)]:
        if pane_id is not None:
            subprocess.run(["tmux", "kill-pane", "-t", pane_id], capture_output=True)
            log(f"[crawler] {label} pane {pane_id} destroyed.")
    _think_pane_id = None
    _snap_pane_id = None


def _update_think_pane(text: str) -> None:
    """Update the left think pane."""
    if _think_pane_id is None:
        return
    try:
        with open(_think_file, "w") as f:
            f.write(text)
    except OSError:
        pass


def _update_snap_pane(text: str) -> None:
    """Update the right snapshot pane."""
    if _snap_pane_id is None:
        return
    try:
        with open(_snap_file, "w") as f:
            f.write(text)
    except OSError:
        pass


# ---------------------------------------------------------------------------
# Launcher
# ---------------------------------------------------------------------------

def _tmux_session_exists(session: str) -> bool:
    return subprocess.run(
        ["tmux", "has-session", "-t", session],
        capture_output=True,
    ).returncode == 0


def _tmux_pane_target(session: str) -> str:
    return f"{session}:0.0"


def launch(session: str, term_w: int, term_h: int, jmoria_path: str, persistent_session: bool = False) -> None:
    """Launch jmoria in tmux, optionally reusing a persistent session."""
    if persistent_session:
        pane_target = _tmux_pane_target(session)
        if not _tmux_session_exists(session):
            shell = os.environ.get("SHELL", "/bin/zsh")
            subprocess.run(
                ["tmux", "new-session", "-d", "-s", session, "-x", str(term_w), "-y", str(term_h), shell],
                check=True,
            )
            time.sleep(0.1)
            if not _tmux_session_exists(session):
                raise RuntimeError(f"Failed to create persistent tmux session '{session}'")
        else:
            subprocess.run(["tmux", "resize-window", "-t", session, "-x", str(term_w), "-y", str(term_h)], check=False)

        subprocess.run(["tmux", "send-keys", "-t", pane_target, "C-c"], check=False)
        time.sleep(0.1)
        subprocess.run(["tmux", "send-keys", "-t", pane_target, "clear", "Enter"], check=True)

        cmdline = f"env JMORIA_SHOW_PLAYER_POS=1 {shlex.quote(jmoria_path)} 2>/tmp/crawler.log"
        subprocess.run(["tmux", "send-keys", "-t", pane_target, cmdline, "Enter"], check=True)
        log(f"[crawler] Launched jmoria in persistent tmux session '{session}' ({term_w}x{term_h})")
        return

    subprocess.run(["tmux", "kill-session", "-t", session], capture_output=True)
    time.sleep(0.2)
    subprocess.run(
        [
            "tmux", "new-session", "-d",
            "-s", session,
            "-x", str(term_w),
            "-y", str(term_h),
            "env",
            "JMORIA_SHOW_PLAYER_POS=1",
            jmoria_path,
        ],
        check=True,
    )
    log(f"[crawler] Launched jmoria in tmux session '{session}' ({term_w}x{term_h})")


def wait_for(predicate, timeout: float = POLL_TIMEOUT, desc: str = "") -> bool:
    """Poll until predicate() returns True or timeout expires."""
    deadline = time.time() + timeout
    while time.time() < deadline:
        lines = screen.get_raw_lines()
        if predicate(lines):
            return True
        time.sleep(POLL_DELAY)
    log(f"[crawler] TIMEOUT waiting for: {desc}")
    return False


# ---------------------------------------------------------------------------
# Startup sequence
# ---------------------------------------------------------------------------

def run_startup() -> bool:
    """Drive through splash and character creation screens into the game."""
    log("[crawler] Waiting for splash screen...")
    if not wait_for(screen.is_splash_screen, desc="splash screen"):
        return False

    log("[crawler] Splash screen detected — pressing SPACE")
    cmd.send(" ")

    log("[crawler] Waiting for character creation screen...")
    if not wait_for(screen.is_char_creation, desc="character creation"):
        return False

    log("[crawler] Character creation screen — pressing SPACE to start game")
    cmd.send(" ")

    log("[crawler] Waiting for dungeon (looking for '@')...")
    def has_player(lines):
        return any("@" in line for line in lines)

    if not wait_for(has_player, timeout=15.0, desc="player '@' in dungeon"):
        return False

    log("[crawler] In dungeon. Bot starting.")
    return True


def replay_init_keys(init_keys_str: str) -> None:
    """Send a sequence of initialization keys before the bot loop starts.

    Keys are comma-separated.  Special tokens:
      C-x    → Ctrl+x (sent as tmux C-x)
      Tab    → Tab key (same as Ctrl+I)
      Enter  → Enter key
      sleep  → pause 1 second
      anything else → individual characters sent one at a time
    """
    if not init_keys_str:
        return
    tokens = [t.strip() for t in init_keys_str.split(",") if t.strip()]
    log(f"[crawler] Sending {len(tokens)} init-key tokens...")
    _send_tokens(tokens)
    log("[crawler] Init keys sent.")


def load_init_file(filepath: str) -> None:
    """Load and replay init keys from a scenario file.

    Scenario file format (one token per line):
      - Lines starting with '#' are comments
      - Blank lines are ignored
      - Each non-comment line is one init-key token (same syntax as --init-keys)
      - Tokens: C-x (Ctrl+x), Tab, Enter, sleep, or literal text

    Example scenario file::

        # Teleport to enter wizard mode
        C-t
        sleep
        # Create a Wand of Light
        Tab
        Wand of Light
        Enter
        sleep
        # Pick it up
        g
        sleep
        # Exit wizard mode
        C-w
        xyzzy
        Enter
    """
    path = Path(filepath)
    if not path.exists():
        log(f"[crawler] Init file not found: {filepath}")
        sys.exit(1)
    tokens = []
    for line in path.read_text().splitlines():
        stripped = line.strip()
        if not stripped or stripped.startswith("#"):
            continue
        tokens.append(stripped)
    log(f"[crawler] Loading {len(tokens)} init tokens from {filepath}")
    _send_tokens(tokens)
    log("[crawler] Init file replay complete.")


def _send_tokens(tokens: list) -> None:
    """Send a list of init-key tokens to the game via tmux."""
    for token in tokens:
        if token == "Enter":
            cmd.send_enter()
        elif token == "Tab":
            cmd.send("Tab")
        elif token == "sleep":
            time.sleep(1.0)
        elif token.startswith("C-") and len(token) == 3:
            cmd.send_ctrl(token[2])
        else:
            # Send each character individually (e.g. "Wand of Light")
            for ch in token:
                cmd.send(ch)
        time.sleep(TICK_DELAY)


# ---------------------------------------------------------------------------
# Main loop
# ---------------------------------------------------------------------------

def run_loop(verbose: bool = False, knowledge_file: str = "", think: bool = False, max_turns: int = 0) -> None:
    depth = 1
    prev_msg = ""
    msg_age_turns = 0
    turn = 0
    engine = DecisionEngine()
    if knowledge_file:
        loaded = engine.load_knowledge(knowledge_file)
        kpath = Path(knowledge_file)
        if not loaded and not kpath.exists():
            kpath.parent.mkdir(parents=True, exist_ok=True)
            engine.save_knowledge(knowledge_file)
    lost_player_turns = 0
    zero_hp_turns = 0
    panel_setup_done = False
    prev_state = None

    while True:
        state = screen.read(state=prev_state, dungeon_depth=depth)
        # #206: Sync local depth from the stats-panel parse so the log
        # reflects the actual depth even when staircase messages are missed.
        depth = state.dungeon_depth
        turn += 1

        if max_turns and turn > max_turns:
            log(f"[crawler] Reached max turns ({max_turns}). Stopping.")
            break

        # Track dungeon depth from messages
        msg = state.last_message
        if msg == prev_msg:
            msg_age_turns += 1
        else:
            msg_age_turns = 1

        if msg != prev_msg:
            if "down staircase" in msg.lower() or "maze of down" in msg.lower():
                depth += 1
                state.dungeon_depth = depth
            elif "up staircase" in msg.lower() or "maze of up" in msg.lower():
                depth = max(1, depth - 1)
                state.dungeon_depth = depth
            prev_msg = msg

        # Detect death. Keep this broad because the UI can change after death,
        # causing the stats parser to return 0/0 and '@' to disappear.
        raw_lower = "\n".join(state.raw_lines).lower()
        if state.player_hp == 0:
            zero_hp_turns += 1
        else:
            zero_hp_turns = 0

        if (
            "tombstone" in raw_lower
            or "you die" in raw_lower
            or ( state.player_max_hp > 0 and state.player_hp == 0 )
            or ( state.player_hp == 0 and state.player_max_hp == 0 and zero_hp_turns >= 3 )
        ):
            log(f"[crawler] Death detected at turn {turn}, depth {depth}. Stopping.")
            log(engine.death_summary(turn, state))
            break

        if not state.is_in_game:
            lost_player_turns += 1
            if lost_player_turns == 1 and verbose:
                log("[crawler] === Screen dump when player lost ===")
                for i, line in enumerate(state.raw_lines):
                    log(f"  [{i:2d}] {line}")
                log("[crawler] === End screen dump ===")
            log(f"[crawler] Lost player '@' at turn {turn} — may be in a menu, skipping")
            if lost_player_turns >= 15:
                log("[crawler] Player missing too long; stopping run.")
                break
            # Dismiss potential --more-- prompts or menus with Space/Escape.
            if lost_player_turns <= 5:
                cmd.send(" ")
            elif lost_player_turns <= 10:
                cmd.send("\x1b")  # Escape key
            time.sleep(POLL_DELAY)
            continue

        lost_player_turns = 0
        prev_state = state

        # One-time panel setup only: inventory is on by default; enable equipment once.
        if not panel_setup_done:
            panels = screen.get_panel_visibility(state.raw_lines)
            if not panels.get("equipment", True):
                if verbose:
                    log(f"[turn {turn:5d}] setup_panel equipment -> 'e'")
                cmd.send("e")
                time.sleep(TICK_DELAY)
            panel_setup_done = True

        action = engine.decide(state)

        if verbose:
            msg_display = msg
            if msg_age_turns > 1:
                msg_display = f"{msg} <same for {msg_age_turns} turns>"
            thought = engine.debug_thought()
            parse_dbg = screen.get_last_parse_debug()
            parse_note = ""
            if (
                len(state.monsters) > 20
                or len(state.items) > 20
                or parse_dbg.get("rejected_monsters", 0) > 0
                or parse_dbg.get("rejected_items", 0) > 0
            ):
                parse_note = (
                    f" parse(rejM={parse_dbg.get('rejected_monsters', 0)}"
                    f",rejI={parse_dbg.get('rejected_items', 0)})"
                )
            log(
                f"[turn {turn:5d}] HP={state.player_hp}/{state.player_max_hp} "
                f"depth={depth} wpos={state.player_world_pos} "
                f"monsters={len(state.monsters)} items={len(state.items)} "
                f"unx={len(engine.unexplored_tiles)} "
                f"think={thought!r}{parse_note} msg={msg_display!r:40s} -> {action!r}"
            )

        # Update thought-bubble pane if enabled.
        if think:
            _update_think_pane(engine.think_status(state, turn))

        if action is None:
            action = "."  # fallback: wait in place

        cmd.send(action)

        # Periodic snapshot every 100 turns.
        if turn > 0 and turn % 100 == 0:
            log(engine.periodic_snapshot(turn, state))
            if think:
                _update_snap_pane(engine.compact_snapshot(turn, state))

        if knowledge_file and engine.knowledge_dirty:
            Path(knowledge_file).parent.mkdir(parents=True, exist_ok=True)
            engine.save_knowledge(knowledge_file)

        time.sleep(TICK_DELAY)

    # Run summary on exit (death or interrupted).
    log(engine.run_summary(turn))


# ---------------------------------------------------------------------------
# Entry point
# ---------------------------------------------------------------------------

def main() -> None:
    parser = argparse.ArgumentParser(description="JMoria dungeon crawler bot")
    parser.add_argument("--session", default="crawler", help="tmux session name")
    parser.add_argument("--term-w", type=int, default=125, help="terminal width")
    parser.add_argument("--term-h", type=int, default=45, help="terminal height")
    parser.add_argument(
        "--jmoria", default="./jmoria", help="path to jmoria executable"
    )
    parser.add_argument("--verbose", "-v", action="store_true")
    parser.add_argument(
        "--think",
        action="store_true",
        help="show bot thought bubble in a tmux status pane",
    )
    parser.add_argument(
        "--no-launch",
        action="store_true",
        help="skip launching jmoria (attach to existing session)",
    )
    parser.add_argument(
        "--persistent-session",
        action="store_true",
        help="reuse and keep tmux session open across bot runs",
    )
    parser.add_argument(
        "--knowledge-file",
        default="scripts/bot/knowledge.json",
        help="path to persistent learned bot knowledge json",
    )
    parser.add_argument(
        "--init-keys",
        default="",
        help="comma-separated keys to send before bot starts (e.g. C-t,Tab,Wand of Light,Enter,g,C-w,xyzzy,Enter)",
    )
    parser.add_argument(
        "--init-file",
        default="",
        help="path to a scenario file with init keys (one token per line, # comments)",
    )
    parser.add_argument(
        "--max-turns",
        type=int,
        default=0,
        help="stop after this many turns (0 = unlimited)",
    )
    args = parser.parse_args()

    # Propagate session name to submodules
    cmd.SESSION = args.session
    screen.SESSION = args.session
    screen.TERM_W = args.term_w
    screen.TERM_H = args.term_h

    if args.knowledge_file:
        kpath = Path(args.knowledge_file)
        if not kpath.exists():
            kpath.parent.mkdir(parents=True, exist_ok=True)
            DecisionEngine().save_knowledge(args.knowledge_file)

    if not args.no_launch:
        launch(
            args.session,
            args.term_w,
            args.term_h,
            args.jmoria,
            persistent_session=args.persistent_session,
        )

    if not run_startup():
        log("[crawler] Failed to reach dungeon. Exiting.")
        sys.exit(1)

    if args.init_file:
        time.sleep(0.5)
        load_init_file(args.init_file)
        time.sleep(0.5)
        lines = screen.get_raw_lines()
        log(f"[crawler] Post-init screen top: {lines[0:3]}")
    elif args.init_keys:
        time.sleep(0.5)
        replay_init_keys(args.init_keys)
        time.sleep(0.5)
        lines = screen.get_raw_lines()
        log(f"[crawler] Post-init screen top: {lines[0:3]}")

    # Create think pane after startup so the game pane is already active.
    think_enabled = args.think
    if think_enabled:
        if not _create_think_pane(args.session):
            log("[crawler] Think pane unavailable; continuing without it.")
            think_enabled = False

    try:
        run_loop(verbose=args.verbose, knowledge_file=args.knowledge_file, think=think_enabled, max_turns=args.max_turns)
    except KeyboardInterrupt:
        log("\n[crawler] Interrupted.")
    finally:
        _destroy_think_pane()
        if not args.no_launch and not args.persistent_session:
            subprocess.run(["tmux", "kill-session", "-t", args.session], capture_output=True)
            log(f"[crawler] tmux session '{args.session}' killed.")
        elif args.persistent_session:
            log(f"[crawler] tmux session '{args.session}' left running (persistent mode).")


if __name__ == "__main__":
    main()
