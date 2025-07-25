const BASE_URL = 'http://localhost:5000';

/**
 * Send the grid, start/end points, and chosen algorithm to the server
 * and return the exploration order and final path.
 *
 * @param {number[][]} grid 2D array (0 = empty, 1 = wall)
 * @param {[number, number]} start [row, col] of the start cell
 * @param {[number, number]} end [row, col] of the end cell
 * @param {string} algorithm One of: 'bfs', 'dfs', 'dijkstra', 'astar', 'bidirectional'
 * @returns {Promise<{ visited: Array<[number, number]>, path: Array<[number, number]> }>} 
 */
export async function solve(grid, start, end, algorithm) {
  const payload = { grid, start, end, algorithm };
  const url = `${BASE_URL}/api/solve`;

  try {
    const response = await fetch(url, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify(payload),
    });

    if (!response.ok) {
      const errorData = await response.json().catch(() => ({}));
      const message = errorData.error || response.statusText;
      throw new Error(`Server error: ${message}`);
    }

    const data = await response.json();
    // Validate the expected fields
    if (!data.visited || !data.path) {
      throw new Error('Invalid response format from server.');
    }

    return {
      visited: data.visited,
      path: data.path,
    };
  } catch (err) {
    console.error('Error in solve():', err);
    throw err;
  }
}
