"""
Bidirectional Search implementation for the pathfinding visualizer.

Functions:
    bidirectional_search(grid, start, end):
        - grid: 2D list of ints where 0 = empty cell, 1 = wall
        - start: tuple (row, col) for the starting cell
        - end: tuple (row, col) for the ending cell
        Returns a tuple (visited_order, path):
            visited_order: list of [row, col] in the order nodes are visited from both searches
            path: list of [row, col] forming the shortest path from start to end (inclusive),
                  or empty list if no path exists
"""
from collections import deque

def bidirectional_search(grid, start, end):
    rows = len(grid)
    cols = len(grid[0]) if rows > 0 else 0

    def in_bounds(r, c):
        return 0 <= r < rows and 0 <= c < cols

    # Validate start/end
    if start == end:
        return [[start[0], start[1]]], [[start[0], start[1]]]
    if not in_bounds(*start) or not in_bounds(*end):
        return [], []
    if grid[start[0]][start[1]] == 1 or grid[end[0]][end[1]] == 1:
        return [], []

    # Initialize frontiers
    f_queue = deque([start])
    b_queue = deque([end])
    f_visited = {start}
    b_visited = {end}
    f_parent = {}
    b_parent = {}
    visited_order = []
    meet_node = None

    # Directions: up, right, down, left
    directions = [(-1, 0), (0, 1), (1, 0), (0, -1)]

    # Bidirectional BFS
    while f_queue and b_queue and meet_node is None:
        # Forward step
        for _ in range(len(f_queue)):
            current = f_queue.popleft()
            visited_order.append([current[0], current[1]])
            for dr, dc in directions:
                nr, nc = current[0] + dr, current[1] + dc
                neighbor = (nr, nc)
                if in_bounds(nr, nc) and neighbor not in f_visited and grid[nr][nc] == 0:
                    f_queue.append(neighbor)
                    f_visited.add(neighbor)
                    f_parent[neighbor] = current
                    if neighbor in b_visited:
                        meet_node = neighbor
                        break
            if meet_node:
                break
        if meet_node:
            break

        # Backward step
        for _ in range(len(b_queue)):
            current = b_queue.popleft()
            visited_order.append([current[0], current[1]])
            for dr, dc in directions:
                nr, nc = current[0] + dr, current[1] + dc
                neighbor = (nr, nc)
                if in_bounds(nr, nc) and neighbor not in b_visited and grid[nr][nc] == 0:
                    b_queue.append(neighbor)
                    b_visited.add(neighbor)
                    b_parent[neighbor] = current
                    if neighbor in f_visited:
                        meet_node = neighbor
                        break
            if meet_node:
                break

    # No connection found
    if meet_node is None:
        return visited_order, []

    # Reconstruct path
    # Forward path from start to meet_node
    path_f = []
    node = meet_node
    while node != start:
        path_f.append([node[0], node[1]])
        node = f_parent.get(node)
        if node is None:
            break
    path_f.append([start[0], start[1]])
    path_f.reverse()

    # Backward path from meet_node to end (exclude meet_node duplicate)
    path_b = []
    node = meet_node
    while node != end:
        node = b_parent.get(node)
        if node is None:
            break
        path_b.append([node[0], node[1]])

    full_path = path_f + path_b
    return visited_order, full_path
