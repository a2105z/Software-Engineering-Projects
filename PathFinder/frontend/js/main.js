import {
  initializeGrid,
  getGridState,
  clearGrid,
  clearAnimations
} from './grid.js';
import { solve } from './api.js';
import { animateSearch } from './animate.js';

// DOM elements
const runBtn = document.getElementById('run-btn');
const clearBtn = document.getElementById('clear-btn');
const algoSelect = document.getElementById('algorithm');

// Initial grid setup
const DEFAULT_ROWS = 60;
const DEFAULT_COLS = 60;
initializeGrid(DEFAULT_ROWS, DEFAULT_COLS);

// Disable UI during processing
function setControlsDisabled(disabled) {
  runBtn.disabled = disabled;
  clearBtn.disabled = disabled;
  algoSelect.disabled = disabled;
}

// Run button handler
runBtn.addEventListener('click', async () => {
  const { grid, start, end } = getGridState();
  const algorithm = algoSelect.value;

  if (!start || !end) {
    alert('Please set both a start and an end point before running.');
    return;
  }

  // Clear only previous animations (visited & path), preserve walls & points
  clearAnimations();
  setControlsDisabled(true);

  try {
    const { visited, path } = await solve(grid, start, end, algorithm);
    await animateSearch(visited, path);
  } catch (err) {
    console.error(err);
    alert(`Error running algorithm: ${err.message}`);
  } finally {
    setControlsDisabled(false);
  }
});

// Clear button handler: fully reset (walls, start, end, and animations)
clearBtn.addEventListener('click', () => {
  clearGrid(false);
});