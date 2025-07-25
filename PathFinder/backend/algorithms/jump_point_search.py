import heapq

def jump_point_search(grid, start, end):
    rows, cols = len(grid), len(grid[0]) if grid else 0

    def in_bounds(r, c):
        return 0 <= r < rows and 0 <= c < cols

    def passable(r, c):
        return in_bounds(r, c) and grid[r][c] == 0

    def heuristic(a, b):
        return abs(a[0]-b[0]) + abs(a[1]-b[1])

    DIRS = [(-1,0),(0,1),(1,0),(0,-1)]

    def jump(r, c, dr, dc):
        nr, nc = r + dr, c + dc
        if not passable(nr, nc):
            return None
        if (nr, nc) == end:
            return (nr, nc)

        # forced neighbor checks
        if dr != 0:
            if (passable(nr, nc+1) and not passable(r, nc+1)) or \
               (passable(nr, nc-1) and not passable(r, nc-1)):
                return (nr, nc)
        elif dc != 0:
            if (passable(nr+1, nc) and not passable(nr, c)) or \
               (passable(nr-1, nc) and not passable(nr, c)):
                return (nr, nc)

        # **only** continue straight—no side‑jumps
        return jump(nr, nc, dr, dc)

    # A* over jump points
    open_heap = []
    g_score = {start: 0}
    f_score = {start: heuristic(start, end)}
    heapq.heappush(open_heap, (f_score[start], 0, start))
    parent, visited, visited_order = {}, set(), []

    while open_heap:
        _, _, current = heapq.heappop(open_heap)
        if current in visited:
            continue
        visited.add(current)
        visited_order.append([current[0], current[1]])
        if current == end:
            break

        for dr, dc in DIRS:
            jp = jump(current[0], current[1], dr, dc)
            if not jp or jp in visited:
                continue
            tentative_g = g_score[current] + heuristic(current, jp)
            if tentative_g < g_score.get(jp, float('inf')):
                parent[jp] = current
                g_score[jp] = tentative_g
                f_score[jp] = tentative_g + heuristic(jp, end)
                heapq.heappush(open_heap, (f_score[jp], tentative_g, jp))

    # reconstruct path
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
