""" Recursive Best‑First Search (RBFS) implementation for the pathfinding visualizer. """

import math
import sys
sys.setrecursionlimit(10000)

def recursive_best_first(grid, start, end):
    rows = len(grid)
    cols = len(grid[0]) if rows > 0 else 0

    def in_bounds(r, c):
        return 0 <= r < rows and 0 <= c < cols

    def is_passable(r, c):
        return in_bounds(r, c) and grid[r][c] == 0

    def heuristic(a, b):
        return abs(a[0] - b[0]) + abs(a[1] - b[1])

    # Validate start/end properly
    if not in_bounds(*start) or not in_bounds(*end):
        return [], []
    if grid[start[0]][start[1]] == 1 or grid[end[0]][end[1]] == 1:
        return [], []

    visited_order = []

    directions = [(-1,0),(0,1),(1,0),(0,-1)]

    def rbfs(node, g, f_limit):
        visited_order.append([node[0], node[1]])
        h = heuristic(node, end)
        f = max(g + h, f_limit)

        if node == end:
            return True, f, [node]

        successors = []
        for dr, dc in directions:
            nr, nc = node[0] + dr, node[1] + dc
            if is_passable(nr, nc):
                neighbor = (nr, nc)
                # note: we don't re‑visit nodes already expanded
                if [nr, nc] not in visited_order:
                    h2 = heuristic(neighbor, end)
                    successors.append([neighbor, max(g + 1 + h2, f)])

        if not successors:
            return False, math.inf, []

        successors.sort(key=lambda x: x[1])

        while successors:
            best, f_best = successors[0]
            if f_best > f_limit:
                return False, f_best, []
            alt = successors[1][1] if len(successors) > 1 else math.inf

            found, new_f, path = rbfs(best, g + 1, min(f_limit, alt))
            successors[0][1] = new_f
            successors.sort(key=lambda x: x[1])
            if found:
                return True, new_f, [node] + path

        return False, math.inf, []

    _, _, full_path = rbfs(start, 0, math.inf)
    # convert to list-of-lists for JSON
    return visited_order, [[r, c] for r, c in full_path]