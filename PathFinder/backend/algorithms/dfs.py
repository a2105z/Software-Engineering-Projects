"""
Depth-First Search (DFS) implementation for the pathfinding visualizer.

Functions:
    dfs(grid, start, end):
        - grid: 2D list of ints where 0 = empty cell, 1 = wall
        - start: tuple (row, col) for the starting cell
        - end: tuple (row, col) for the ending cell
        Returns a tuple (visited_order, path):
            visited_order: list of [row, col] in the order nodes are visited
            path: list of [row, col] forming a path from start to end (inclusive),
                  or empty list if no path exists
Note: DFS does not guarantee shortest path in unweighted graphs, but will return a valid path.
"""

def dfs(grid, start, end):
    rows = len(grid)
    cols = len(grid[0]) if rows > 0 else 0

    def in_bounds(r, c):
        return 0 <= r < rows and 0 <= c < cols

    # Validate start/end
    if not in_bounds(*start) or not in_bounds(*end):
        return [], []
    if grid[start[0]][start[1]] == 1 or grid[end[0]][end[1]] == 1:
        return [], []

    visited = set()
    parent = {}
    visited_order = []

    # Stack for DFS
    stack = [start]
    visited.add(start)

    # Directions: up, right, down, left
    directions = [(-1, 0), (0, 1), (1, 0), (0, -1)]

    found = False
    while stack:
        current = stack.pop()
        visited_order.append([current[0], current[1]])

        if current == end:
            found = True
            break

        # Explore neighbors
        for dr, dc in directions:
            nr, nc = current[0] + dr, current[1] + dc
            neighbor = (nr, nc)
            if in_bounds(nr, nc) and neighbor not in visited and grid[nr][nc] == 0:
                stack.append(neighbor)
                visited.add(neighbor)
                parent[neighbor] = current

    # Reconstruct path if found
    path = []
    if found:
        node = end
        while node != start:
            path.append([node[0], node[1]])
            node = parent.get(node)
            if node is None:
                break
        # Add start
        path.append([start[0], start[1]])
        path.reverse()

    return visited_order, path
