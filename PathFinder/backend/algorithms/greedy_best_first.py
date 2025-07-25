"""
Greedy Best-First Search (GBFS) implementation for the pathfinding visualizer.

Functions:
    greedy_best_first(grid, start, end):
        - grid: 2D list of ints where 0 = empty cell, 1 = wall
        - start: tuple (row, col) for the starting cell
        - end: tuple (row, col) for the ending cell
        Returns a tuple (visited_order, path):
            visited_order: list of [row, col] in the order nodes are dequeued
            path: list of [row, col] forming the path from start to end (inclusive),
                  or empty list if no path exists
Uses only the heuristic to guide the search (no accumulated cost).
"""

import heapq

def greedy_best_first(grid, start, end):
    rows = len(grid)
    cols = len(grid[0]) if rows > 0 else 0

    def in_bounds(r, c):
        return 0 <= r < rows and 0 <= c < cols

    def heuristic(a, b):
        # Manhattan distance
        return abs(a[0] - b[0]) + abs(a[1] - b[1])

    # Validate start/end
    if not in_bounds(*start) or not in_bounds(*end):
        return [], []
    if grid[start[0]][start[1]] == 1 or grid[end[0]][end[1]] == 1:
        return [], []

    # Min-heap priority queue: (h_score, count, node)
    open_heap = []
    count = 0
    start_h = heuristic(start, end)
    heapq.heappush(open_heap, (start_h, count, start))
    visited = set()
    parent = {}
    visited_order = []

    # Neighbor directions: up, right, down, left
    directions = [(-1,0),(0,1),(1,0),(0,-1)]

    while open_heap:
        _, _, current = heapq.heappop(open_heap)
        if current in visited:
            continue

        visited.add(current)
        visited_order.append([current[0], current[1]])

        if current == end:
            break

        for dr, dc in directions:
            nr, nc = current[0] + dr, current[1] + dc
            neighbor = (nr, nc)
            if in_bounds(nr, nc) and neighbor not in visited and grid[nr][nc] == 0:
                parent[neighbor] = current
                count += 1
                h = heuristic(neighbor, end)
                heapq.heappush(open_heap, (h, count, neighbor))

    # Reconstruct path
    path = []
    if end in parent or start == end:
        node = end
        while node != start:
            path.append([node[0], node[1]])
            node = parent.get(node)
            if node is None:
                break
        path.append([start[0], start[1]])
        path.reverse()

    return visited_order, path
