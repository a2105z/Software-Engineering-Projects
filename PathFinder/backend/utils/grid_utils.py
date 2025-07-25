"""
Utility functions for validating and parsing the grid and point data
received from the front end payload.
"""

def validate_grid(grid):
    """
    Validate that `grid` is a non-empty 2D list of 0s and 1s,
    rectangular in shape (all rows same length).
    """

    if not isinstance(grid, list) or not grid:
        raise ValueError("Grid must be a non-empty 2D list of 0s and 1s.")
    row_length = None
    for idx, row in enumerate(grid):
        if not isinstance(row, list):
            raise ValueError(f"Row {idx} in grid is not a list.")
        if row_length is None:
            row_length = len(row)
            if row_length == 0:
                raise ValueError("Each row in grid must have at least one column.")
        elif len(row) != row_length:
            raise ValueError(f"Row {idx} length {len(row)} does not match expected {row_length}.")
        for jdx, cell in enumerate(row):
            if not isinstance(cell, int) or cell not in (0, 1):
                raise ValueError(f"Grid cell at ({idx},{jdx}) is not 0 or 1.")
    return grid


def validate_point(point, grid, name="point"):
    """
    Validate that `point` is a list or tuple of two integers within grid bounds
    and not a wall (cell value 1).
    """

    if not isinstance(point, (list, tuple)) or len(point) != 2:
        raise ValueError(f"{name} must be a list or tuple of two integers.")
    r, c = point

    if not isinstance(r, int) or not isinstance(c, int):
        raise ValueError(f"{name} coordinates must be integers, got {type(r)} and {type(c)}.")
    
    max_r = len(grid)
    max_c = len(grid[0]) if max_r > 0 else 0

    if r < 0 or r >= max_r or c < 0 or c >= max_c:
        raise ValueError(f"{name} {point} is out of grid bounds (0 <= row < {max_r}, 0 <= col < {max_c}).")
    
    if grid[r][c] == 1:
        raise ValueError(f"{name} {point} is a wall cell.")
    
    return (r, c)


def parse_payload(data, supported_algorithms=None):
    """ Parse and validate the incoming JSON payload for the solve endpoint. """
    
    if not isinstance(data, dict):
        raise ValueError("Payload must be a JSON object.")

    # Validate grid
    if "grid" not in data:
        raise ValueError("Missing 'grid' field in payload.")
    grid = validate_grid(data["grid"])

    # Validate start
    if "start" not in data:
        raise ValueError("Missing 'start' field in payload.")
    start = validate_point(data["start"], grid, name="start")

    # Validate end
    if "end" not in data:
        raise ValueError("Missing 'end' field in payload.")
    end = validate_point(data["end"], grid, name="end")

    # Algorithm
    algo = data.get("algorithm", None)
    if supported_algorithms is not None:
        if not isinstance(algo, str) or algo.lower() not in supported_algorithms:
            raise ValueError(f"Invalid algorithm '{algo}'. Supported: {supported_algorithms}.")
        algo = algo.lower()
    return grid, start, end, algo
