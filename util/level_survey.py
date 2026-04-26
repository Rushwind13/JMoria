#!/usr/bin/env python3
"""
level_survey.py — per-10-level coverage + per-IDX spread survey
for Items.txt and Monsters.txt.

Spawn window: an entry with Level L can appear on dungeon levels
max(1, L-5) .. L+5  (based on +/- 5 spawn routine).
"""

import re
import sys
from collections import defaultdict

SPAWN_RADIUS = 5
MAX_LEVEL    = 100
BUCKET_SIZE  = 10
BUCKETS      = list(range(1, MAX_LEVEL + 1, BUCKET_SIZE))  # 1,11,21,...91

def bucket_label(b):
    return f"{b:2d}-{b+BUCKET_SIZE-1:2d}"

# ──────────────────────────────────────────────────────────────────────────────

def parse_entries(path):
    """Return list of dicts with name, type_idx, level."""
    text = open(path).read()
    entries = []
    for m in re.finditer(r'^(?:Item|Monster)\s+<([^>]+)>', text, re.MULTILINE):
        name = m.group(1)
        # grab the block
        start = m.start()
        brace = text.find('{', start)
        depth, i = 0, brace
        while i < len(text):
            if text[i] == '{': depth += 1
            elif text[i] == '}':
                depth -= 1
                if depth == 0:
                    break
            i += 1
        block = text[start:i+1]
        type_m  = re.search(r'^\s*Type\s+<([^>]+)>',  block, re.MULTILINE)
        level_m = re.search(r'^\s*Level\s+(\d+)',      block, re.MULTILINE)
        entries.append({
            'name':  name,
            'type':  type_m.group(1)  if type_m  else '???',
            'level': int(level_m.group(1)) if level_m else 0,
        })
    return entries


def spawn_range(level):
    lo = max(1, level - SPAWN_RADIUS)
    hi = level + SPAWN_RADIUS
    return lo, hi


def buckets_covered(level):
    lo, hi = spawn_range(level)
    covered = []
    for b in BUCKETS:
        b_hi = b + BUCKET_SIZE - 1
        if lo <= b_hi and hi >= b:
            covered.append(b)
    return covered


# ──────────────────────────────────────────────────────────────────────────────

def survey(path, kind):
    entries = parse_entries(path)
    print(f"\n{'═'*72}")
    print(f"  {kind.upper()} SURVEY  —  {path.split('/')[-1]}  ({len(entries)} entries)")
    print(f"{'═'*72}")

    # ── per-IDX spread ──────────────────────────────────────────────────────
    by_type = defaultdict(list)
    for e in entries:
        by_type[e['type']].append(e['level'])

    print(f"\n{'── Per-IDX Spread ':─<72}")
    print(f"{'IDX':<30}  {'Count':>5}  {'Min':>4}  {'Max':>4}  {'Spawn window'}")
    print(f"{'─'*30}  {'─'*5}  {'─'*4}  {'─'*4}  {'─'*20}")
    for idx in sorted(by_type):
        levels = sorted(by_type[idx])
        lo_spawn = max(1, min(levels) - SPAWN_RADIUS)
        hi_spawn = max(levels) + SPAWN_RADIUS
        bar_lo = (lo_spawn - 1) // BUCKET_SIZE
        bar_hi = (hi_spawn - 1) // BUCKET_SIZE
        span = '·' * bar_lo + '█' * (bar_hi - bar_lo + 1) + '·' * (9 - bar_hi)
        print(f"{idx:<30}  {len(levels):>5}  {min(levels):>4}  {max(levels):>4}  {span}  (lvl {lo_spawn}–{hi_spawn})")

    # ── per-10-level bucket coverage ────────────────────────────────────────
    print(f"\n{'── Per-10-Level Bucket Coverage ':─<72}")
    bucket_entries = defaultdict(list)   # bucket -> list of (type, name, level)
    for e in entries:
        for b in buckets_covered(e['level']):
            bucket_entries[b].append(e)

    # which IDXs appear in each bucket
    all_idxs = sorted(by_type.keys())

    # header
    idx_col = 22
    hdr = f"{'Bucket':<9}  {'#':>4}  {'IDXs present'}"
    print(hdr)
    print('─' * 72)
    for b in BUCKETS:
        elist = bucket_entries[b]
        count = len(elist)
        idxs_here = sorted({e['type'] for e in elist})
        # short names: strip common prefix for display
        prefix = 'ITEM_IDX_' if kind == 'items' else 'MON_IDX_'
        short = ', '.join(i.replace(prefix, '') for i in idxs_here)
        missing = sorted({i for i in all_idxs if i not in idxs_here})
        miss_short = ', '.join(m.replace(prefix, '') for m in missing)
        flag = '  ← SPARSE' if count < 5 else ''
        print(f"  {bucket_label(b)}   {count:>4}  [{short}]{flag}")
        if missing:
            print(f"{'':>16}  missing: [{miss_short}]")

    # ── density histogram ───────────────────────────────────────────────────
    print(f"\n{'── Density Histogram (entries per bucket, spawn-window weighted) ':─<72}")
    max_count = max((len(bucket_entries[b]) for b in BUCKETS), default=1)
    bar_width = 40
    for b in BUCKETS:
        count = len(bucket_entries[b])
        bar = '█' * int(bar_width * count / max_count)
        print(f"  {bucket_label(b)}  {count:>4}  {bar}")


# ──────────────────────────────────────────────────────────────────────────────

base = '/Users/jimbo/Documents/code/JMoria/Resources'
survey(f'{base}/Items.txt',    'items')
survey(f'{base}/Monsters.txt', 'monsters')
