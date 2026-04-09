"""pathfinding.py - Grid navigation utilities for the JMoria ASCII bot."""

from heapq import heappop, heappush

DIRS_8 = [
    (-1, -1), (-1, 0), (-1, 1),
    (0, -1), (0, 1),
    (1, -1), (1, 0), (1, 1),
]

DIR_TO_KEY = {
    (-1, -1): "y", (-1, 0): "k", (-1, 1): "u",
    (0, -1): "h", (0, 1): "l",
    (1, -1): "b", (1, 0): "j", (1, 1): "n",
}

WALKABLE = {".", "'", "<", ">", "@"}
SOLID = {"#", ":", "+"}

# Unknown tiles (" ") are neither WALKABLE nor SOLID — they are
# traversable but expensive, representing unseen territory that
# *might* be floor.
UNKNOWN_COST = 5


def in_bounds(grid, pos):
    r, c = pos
    if r < 0 or c < 0:
        return False
    if r >= len(grid):
        return False
    if c >= len(grid[0]):
        return False
    return True


def is_walkable(ch):
    if ch in WALKABLE:
        return True
    if ch in SOLID:
        return False
    # Non-empty, non-solid chars can still be passable floor overlays.
    return ch != " "


def is_passable(ch):
    """True if A* may route through this tile (walkable or unknown)."""
    return ch not in SOLID


def neighbors(grid, pos, allow_unknown=False):
    r, c = pos
    out = []
    for dr, dc in DIRS_8:
        nr, nc = r + dr, c + dc
        if in_bounds(grid, (nr, nc)):
            ch = grid[nr][nc]
            if allow_unknown:
                if is_passable(ch):
                    out.append((nr, nc))
            else:
                if is_walkable(ch):
                    out.append((nr, nc))
    return out


def heuristic(a, b):
    # Chebyshev distance for 8-direction movement
    return max(abs(a[0] - b[0]), abs(a[1] - b[1]))


def path_to(grid, start, goal, cost_fn=None):
    """Return path [start..goal] using A*. Empty list if unreachable.

    Routes through unknown (' ') tiles with a high cost penalty so the
    bot prefers known paths but can traverse dark rooms to reach exits.

    cost_fn: optional callable(pos) -> extra cost for stepping onto pos.
    """
    if start == goal:
        return [start]
    if not in_bounds(grid, start) or not in_bounds(grid, goal):
        return []

    open_heap = []
    heappush(open_heap, (0, start))
    came_from = {}
    g_score = {start: 0}

    while open_heap:
        _, cur = heappop(open_heap)
        if cur == goal:
            return _reconstruct_path(came_from, cur)

        for nxt in neighbors(grid, cur, allow_unknown=True):
            ch = grid[nxt[0]][nxt[1]]
            step_cost = 1
            if ch == " ":
                step_cost += UNKNOWN_COST
            if cost_fn:
                step_cost += cost_fn(nxt)
            tentative = g_score[cur] + step_cost
            if tentative < g_score.get(nxt, 1_000_000_000):
                came_from[nxt] = cur
                g_score[nxt] = tentative
                f = tentative + heuristic(nxt, goal)
                heappush(open_heap, (f, nxt))

    return []


def _reconstruct_path(came_from, cur):
    p = [cur]
    while cur in came_from:
        cur = came_from[cur]
        p.append(cur)
    p.reverse()
    return p


def first_step_key(path):
    """Convert a path [start, step1, ...] to a movement key."""
    if len(path) < 2:
        return None
    (r0, c0), (r1, c1) = path[0], path[1]
    dr, dc = r1 - r0, c1 - c0
    return DIR_TO_KEY.get((dr, dc))


def find_nearest_target(grid, start, predicate, cost_fn=None):
    """BFS/Dijkstra nearest tile matching predicate(ch, pos).

    cost_fn: optional callable(pos) -> extra cost for stepping onto pos.
    When provided, uses Dijkstra instead of BFS to prefer low-cost paths.
    """
    if cost_fn:
        from heapq import heappush, heappop
        open_heap = [(0, start)]
        g_score = {start: 0}
        while open_heap:
            cost, cur = heappop(open_heap)
            r, c = cur
            if predicate(grid[r][c], cur):
                return cur
            for nxt in neighbors(grid, cur):
                step_cost = 1 + cost_fn(nxt)
                tentative = g_score[cur] + step_cost
                if tentative < g_score.get(nxt, 1_000_000_000):
                    g_score[nxt] = tentative
                    heappush(open_heap, (tentative, nxt))
        return None

    from collections import deque

    q = deque([start])
    seen = {start}

    while q:
        cur = q.popleft()
        r, c = cur
        if predicate(grid[r][c], cur):
            return cur
        for nxt in neighbors(grid, cur):
            if nxt in seen:
                continue
            seen.add(nxt)
            q.append(nxt)
    return None


def key_away_from(pos, threat_pos, grid):
    """Pick a neighboring move that increases distance from threat."""
    best = None
    best_dist = -1
    for nxt in neighbors(grid, pos):
        d = heuristic(nxt, threat_pos)
        if d > best_dist:
            best_dist = d
            best = nxt
    if best is None:
        return None
    dr, dc = best[0] - pos[0], best[1] - pos[1]
    return DIR_TO_KEY.get((dr, dc))
