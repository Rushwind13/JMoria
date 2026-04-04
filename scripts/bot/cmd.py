"""
cmd.py — Send keystrokes to the jmoria tmux session via tmux send-keys.
"""

import subprocess
import time

SESSION = "crawler"
RATE_LIMIT = 0.05  # seconds between commands (50ms)


def send(key: str) -> None:
    """Send a single keystroke. Do NOT append Enter — game reads single chars."""
    subprocess.run(
        ["tmux", "send-keys", "-t", SESSION, key],
        check=True,
    )
    time.sleep(RATE_LIMIT)


def send_ctrl(key: str) -> None:
    """Send Ctrl+<key>, e.g. send_ctrl('t') for ^t (teleport)."""
    subprocess.run(
        ["tmux", "send-keys", "-t", SESSION, f"C-{key}"],
        check=True,
    )
    time.sleep(RATE_LIMIT)


def send_enter() -> None:
    """Send Enter (used for multi-char input prompts)."""
    subprocess.run(
        ["tmux", "send-keys", "-t", SESSION, "Enter"],
        check=True,
    )
    time.sleep(RATE_LIMIT)
