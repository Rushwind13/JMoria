---
description: Show current JMoria game state from most recent session
allowed-tools: Bash(./ai-scripts/show-state.sh:*)
argument-hint: [session-name]
---

Show the current game state from a JMoria session.

## Session (optional)

$ARGUMENTS

## Current state

!`./ai-scripts/show-state.sh $ARGUMENTS 2>&1`

## Your task

Describe the current game state:
- Player position and health
- Visible monsters and their threat level
- Items on the ground worth picking up
- Tactical situation (safe, dangerous, critical)
