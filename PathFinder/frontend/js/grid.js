// Manages the grid creation, user interactions for setting start/end/walls, and exposes state getters.

let rows = 20;
let cols = 20;
let gridElements = []; // 2D array of cell divs
let startPos = null;   // [r, c]
let endPos = null;     // [r, c]
let isMouseDown = false;
let currentAction = null; // 'wall' or 'erase'

const gridContainer = document.getElementById('grid');

document.body.addEventListener('mouseup', () => {
  isMouseDown = false;
  currentAction = null;
});

document.body.addEventListener('mouseleave', () => {
  isMouseDown = false;
  currentAction = null;
});

function createCell(r, c) {
  const cell = document.createElement('div');
  cell.classList.add('cell', 'empty');
  cell.dataset.row = r;
  cell.dataset.col = c;
  // Disable context menu for right-click
  cell.addEventListener('contextmenu', (e) => e.preventDefault());

  // Mouse down
  cell.addEventListener('mousedown', (e) => {
    e.preventDefault();
    const isLeft = e.button === 0;
    const isRight = e.button === 2;
    const cls = cell.classList;
    const rr = parseInt(cell.dataset.row, 10);
    const cc = parseInt(cell.dataset.col, 10);

    if (isLeft) {
      if (!startPos && !cls.contains('end')) {
        cls.replace('empty', 'start');
        startPos = [rr, cc];
      } else if (!endPos && !cls.contains('start')) {
        cls.replace('empty', 'end');
        endPos = [rr, cc];
      } else if (!cls.contains('start') && !cls.contains('end')) {
        // toggle wall
        if (cls.contains('wall')) {
          cls.replace('wall', 'empty');
          currentAction = 'erase';
        } else {
          cls.replace('empty', 'wall');
          currentAction = 'wall';
        }
      }
    } else if (isRight) {
      // Erase anything
      if (cls.contains('start')) {
        cls.replace('start', 'empty');
        startPos = null;
      } else if (cls.contains('end')) {
        cls.replace('end', 'empty');
        endPos = null;
      } else if (cls.contains('wall')) {
        cls.replace('wall', 'empty');
      }
      currentAction = 'erase';
    }
    isMouseDown = true;
  });

  // Mouse enter for drag
  cell.addEventListener('mouseenter', () => {
    if (!isMouseDown || !currentAction) return;
    const cls = cell.classList;
    if (currentAction === 'wall') {
      if (!cls.contains('start') && !cls.contains('end') && !cls.contains('wall')) {
        cls.replace('empty', 'wall');
      }
    } else if (currentAction === 'erase') {
      if (cls.contains('wall')) {
        cls.replace('wall', 'empty');
      }
    }
  });

  return cell;
}

/**
 * Initialize the grid in the container.
 * @param {number} r Number of rows (default 20)
 * @param {number} c Number of columns (default 20)
 */
export function initializeGrid(r = 20, c = 20) {
  rows = r;
  cols = c;
  gridContainer.innerHTML = '';
  gridContainer.classList.remove('animating');
  gridElements = [];
  startPos = null;
  endPos = null;

  // Set CSS grid properties
  gridContainer.style.gridTemplateColumns = `repeat(${cols}, 1rem)`;
  gridContainer.style.gridTemplateRows = `repeat(${rows}, 1rem)`;

  for (let i = 0; i < rows; i++) {
    const rowArr = [];
    for (let j = 0; j < cols; j++) {
      const cell = createCell(i, j);
      gridContainer.appendChild(cell);
      rowArr.push(cell);
    }
    gridElements.push(rowArr);
  }
}

/**
 * Returns the current grid state as numbers, plus start/end positions.
 */
export function getGridState() {
  const grid = gridElements.map((row) =>
    row.map((cell) => (cell.classList.contains('wall') ? 1 : 0))
  );
  return {
    grid,
    start: startPos,
    end: endPos,
  };
}

/**
 * Clears only animation classes (visited & path), preserving walls/start/end.
 */
export function clearAnimations() {
  gridContainer.classList.remove('animating');
  gridElements.forEach((row) => {
    row.forEach((cell) => {
      cell.classList.remove('visited', 'path');
    });
  });
}

/**
 * Clears the grid completely: removes walls, start, end, and animations.
 * If preservePoints=false, also clears start and end.
 */
export function clearGrid(preservePoints = false) {
  gridContainer.classList.remove('animating');
  gridElements.forEach((row) => {
    row.forEach((cell) => {
      cell.className = 'cell empty';
    });
  });
  if (!preservePoints) {
    startPos = null;
    endPos = null;
  }
}
