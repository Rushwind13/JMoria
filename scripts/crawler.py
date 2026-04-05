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
# Launcher
# ---------------------------------------------------------------------------

def launch(session: str, term_w: int, term_h: int, jmoria_path: str) -> None:
    """Kill any old session and start a fresh one."""
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


# ---------------------------------------------------------------------------
# Main loop
# ---------------------------------------------------------------------------

def run_loop(verbose: bool = False) -> None:
    depth = 1
    prev_msg = ""
    msg_age_turns = 0
    turn = 0
    engine = DecisionEngine()
    lost_player_turns = 0
    zero_hp_turns = 0

    while True:
        state = screen.read(dungeon_depth=depth)
        turn += 1

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
            break

        if not state.is_in_game:
            lost_player_turns += 1
            log(f"[crawler] Lost player '@' at turn {turn} — may be in a menu, skipping")
            if lost_player_turns >= 15:
                log("[crawler] Player missing too long; stopping run.")
                break
            time.sleep(POLL_DELAY)
            continue

        lost_player_turns = 0

        action = engine.decide(state)

        if verbose:
            msg_display = msg
            if msg_age_turns > 1:
                msg_display = f"{msg} <same for {msg_age_turns} turns>"
            thought = engine.debug_thought()
            log(
                f"[turn {turn:5d}] HP={state.player_hp}/{state.player_max_hp} "
                f"depth={depth} wpos={state.player_world_pos} "
                f"monsters={len(state.monsters)} items={len(state.items)} "
                f"think={thought!r} msg={msg_display!r:40s} -> {action!r}"
            )

        cmd.send(action)
        time.sleep(TICK_DELAY)


# ---------------------------------------------------------------------------
# Entry point
# ---------------------------------------------------------------------------

def main() -> None:
    parser = argparse.ArgumentParser(description="JMoria dungeon crawler bot")
    parser.add_argument("--session", default="crawler", help="tmux session name")
    parser.add_argument("--term-w", type=int, default=125, help="terminal width")
    parser.add_argument("--term-h", type=int, default=40, help="terminal height")
    parser.add_argument(
        "--jmoria", default="./jmoria", help="path to jmoria executable"
    )
    parser.add_argument("--verbose", "-v", action="store_true")
    parser.add_argument(
        "--no-launch",
        action="store_true",
        help="skip launching jmoria (attach to existing session)",
    )
    args = parser.parse_args()

    # Propagate session name to submodules
    cmd.SESSION = args.session
    screen.SESSION = args.session
    screen.TERM_W = args.term_w
    screen.TERM_H = args.term_h

    if not args.no_launch:
        launch(args.session, args.term_w, args.term_h, args.jmoria)

    if not run_startup():
        log("[crawler] Failed to reach dungeon. Exiting.")
        sys.exit(1)

    try:
        run_loop(verbose=args.verbose)
    except KeyboardInterrupt:
        log("\n[crawler] Interrupted.")
    finally:
        if not args.no_launch:
            subprocess.run(["tmux", "kill-session", "-t", args.session], capture_output=True)
            log(f"[crawler] tmux session '{args.session}' killed.")


if __name__ == "__main__":
    main()
