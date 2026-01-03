---
description: Instructions for live monitoring a JMoria game
allowed-tools: Bash(ls:*)
---

## Live Monitoring Setup

To monitor a JMoria game in real-time, run one of these in a separate terminal:

```bash
# Brief mode - one-line summaries
./ai-scripts/monitor-live.sh brief

# All events (verbose)
./ai-scripts/monitor-live.sh

# Combat only
./ai-scripts/monitor-live.sh combat

# Player movement only
./ai-scripts/monitor-live.sh player

# Monster movement only
./ai-scripts/monitor-live.sh monster
```

## Current session file

!`ls -t ai-logs/*.log 2>/dev/null | head -1 || echo "No session files found"`

## Your task

Explain how to set up live monitoring. The user should:
1. Start the game with `JMORIA_AI_LOG=1 ./jmoria`
2. In another terminal, run the monitor script
3. Play the game and watch events stream in real-time
