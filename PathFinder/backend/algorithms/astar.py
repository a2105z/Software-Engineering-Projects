"""
A* (A-Star) algorithm implementation for the pathfinding visualizer.

Functions:
    astar(grid, start, end):
        - grid: 2D list of ints where 0 = empty cell, 1 = wall
        - start: tuple (row, col) for the starting cell
        - end: tuple (row, col) for the ending cell
        Returns a tuple (visited_order, path):
            visited_order: list of [row, col] in the order nodes are dequeued (first visit)
            path: list of [row, col] forming the shortest path from start to end (inclusive),
                  or empty list if no path exists

Uses Manhattan distance as the heuristic.
"""
import heapq


def astar(grid, start, end):
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

    # g_score: cost from start to node
    g_score = { (r, c): float('inf') for r in range(rows) for c in range(cols) }
    g_score[start] = 0

    # f_score: g_score + heuristic
    f_score = { (r, c): float('inf') for r in range(rows) for c in range(cols) }
    f_score[start] = heuristic(start, end)

    # Priority queue: (f_score, count, node)
    open_heap = []
    heapq.heappush(open_heap, (f_score[start], 0, start))
    open_set = {start}

    parent = {}
    visited = set()
    visited_order = []
    count = 0

    # Neighbor directions: up, right, down, left
    directions = [(-1, 0), (0, 1), (1, 0), (0, -1)]

    while open_heap:
        _, _, current = heapq.heappop(open_heap)
        open_set.discard(current)

        # Skip if already visited
        if current in visited:
            continue

        visited.add(current)
        visited_order.append([current[0], current[1]])

        # Check if reached end
        if current == end:
            break

        # Explore neighbors
        for dr, dc in directions:
            nr, nc = current[0] + dr, current[1] + dc
            neighbor = (nr, nc)
            if in_bounds(nr, nc) and neighbor not in visited and grid[nr][nc] == 0:
                tentative_g = g_score[current] + 1
                if tentative_g < g_score[neighbor]:
                    parent[neighbor] = current
                    g_score[neighbor] = tentative_g
                    f_score[neighbor] = tentative_g + heuristic(neighbor, end)
                    if neighbor not in open_set:
                        count += 1
                        heapq.heappush(open_heap, (f_score[neighbor], count, neighbor))
                        open_set.add(neighbor)

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
