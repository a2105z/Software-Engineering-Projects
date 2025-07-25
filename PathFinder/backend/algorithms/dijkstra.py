"""
Dijkstra's algorithm implementation for the pathfinding visualizer.

Functions:
    dijkstra(grid, start, end):
        - grid: 2D list of ints where 0 = empty cell, 1 = wall
        - start: tuple (row, col) for the starting cell
        - end: tuple (row, col) for the ending cell
        Returns a tuple (visited_order, path):
            visited_order: list of [row, col] in the order nodes are dequeued (first visit)
            path: list of [row, col] forming the shortest path from start to end (inclusive),
                  or empty list if no path exists

All edges are assumed to have weight 1.
"""

import heapq


def dijkstra(grid, start, end):
    rows = len(grid)
    cols = len(grid[0]) if rows > 0 else 0

    def in_bounds(r, c):
        return 0 <= r < rows and 0 <= c < cols

    # Validate start/end
    if not in_bounds(*start) or not in_bounds(*end):
        return [], []
    if grid[start[0]][start[1]] == 1 or grid[end[0]][end[1]] == 1:
        return [], []

    # Distance to each node; default to infinity
    distances = { (r, c): float('inf') for r in range(rows) for c in range(cols) }
    distances[start] = 0

    # Parent pointers for path reconstruction
    parent = {}

    # Min-heap priority queue: (distance, (r, c))
    heap = []
    heapq.heappush(heap, (0, start))

    visited = set()
    visited_order = []

    # Neighbor directions: up, right, down, left
    directions = [(-1, 0), (0, 1), (1, 0), (0, -1)]

    while heap:
        dist, current = heapq.heappop(heap)

        # Skip if we've already visited
        if current in visited:
            continue

        visited.add(current)
        visited_order.append([current[0], current[1]])

        # Stop if we reached the end
        if current == end:
            break

        # Explore neighbors
        for dr, dc in directions:
            nr, nc = current[0] + dr, current[1] + dc
            neighbor = (nr, nc)
            if in_bounds(nr, nc) and neighbor not in visited and grid[nr][nc] == 0:
                new_dist = dist + 1  # all edges weight=1
                if new_dist < distances[neighbor]:
                    distances[neighbor] = new_dist
                    parent[neighbor] = current
                    heapq.heappush(heap, (new_dist, neighbor))

    # Reconstruct shortest path
    path = []
    if end in parent or start == end:
        # Walk from end to start
        node = end
        while node != start:
            path.append([node[0], node[1]])
            node = parent.get(node)
            if node is None:
                break
        path.append([start[0], start[1]])
        path.reverse()

    return visited_order, path
