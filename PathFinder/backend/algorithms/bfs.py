"""
Breadth-First Search (BFS) implementation for the pathfinding visualizer.

Functions:
    bfs(grid, start, end):
        - grid: 2D list of ints where 0 = empty cell, 1 = wall
        - start: tuple (row, col) for the starting cell
        - end: tuple (row, col) for the ending cell
        Returns a tuple (visited_order, path):
            visited_order: list of [row, col] in the order nodes are visited
            path: list of [row, col] forming the shortest path from start to end (inclusive),
                  or empty list if no path exists
"""

from collections import deque

def bfs(grid, start, end):
    # Dimensions
    rows = len(grid)
    cols = len(grid[0]) if rows > 0 else 0

    # Helper to check valid cell
    def in_bounds(r, c):
        return 0 <= r < rows and 0 <= c < cols

    # If start or end is invalid or blocked
    if not in_bounds(*start) or not in_bounds(*end):
        return [], []
    if grid[start[0]][start[1]] == 1 or grid[end[0]][end[1]] == 1:
        return [], []

    visited = set()
    parent = {}
    visited_order = []

    # Queue for BFS
    queue = deque()
    queue.append(start)
    visited.add(start)

    # Directions: up, right, down, left
    directions = [(-1, 0), (0, 1), (1, 0), (0, -1)]

    found = False
    while queue:
        current = queue.popleft()
        visited_order.append([current[0], current[1]])

        if current == end:
            found = True
            break

        for dr, dc in directions:
            nr, nc = current[0] + dr, current[1] + dc
            neighbor = (nr, nc)
            if in_bounds(nr, nc) and neighbor not in visited and grid[nr][nc] == 0:
                queue.append(neighbor)
                visited.add(neighbor)
                parent[neighbor] = current

    # Reconstruct path if end was reached
    path = []
    if found:
        node = end
        while node != start:
            path.append([node[0], node[1]])
            node = parent.get(node)
            if node is None:
                break
        # Add the start at the beginning
        path.append([start[0], start[1]])
        path.reverse()

    return visited_order, path