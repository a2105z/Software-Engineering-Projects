// Animates the search process: visited nodes then final path

const VISIT_DELAY = 20; // ms per visited node
const PATH_DELAY = 50;  // ms per path node

/**
 * Animate visited nodes and then the final path on the grid.
 *
 * @param {Array<[number, number]>} visited List of [row, col] visited in order
 * @param {Array<[number, number]>} path List of [row, col] forming the final path
 * @returns {Promise<void>} Resolves when animation is complete
 */
export function animateSearch(visited, path) {
  return new Promise((resolve) => {
    const gridContainer = document.getElementById('grid');
    gridContainer.classList.add('animating');

    // Animate visited nodes
    visited.forEach(([r, c], i) => {
      setTimeout(() => {
        const cell = document.querySelector(`.cell[data-row='${r}'][data-col='${c}']`);
        if (cell && !cell.classList.contains('start') && !cell.classList.contains('end')) {
          cell.classList.add('visited');
        }
      }, VISIT_DELAY * i);
    });

    // After visited animation, animate path
    const totalVisitTime = VISIT_DELAY * visited.length;
    path.forEach(([r, c], i) => {
      setTimeout(() => {
        const cell = document.querySelector(`.cell[data-row='${r}'][data-col='${c}']`);
        if (cell && !cell.classList.contains('start') && !cell.classList.contains('end')) {
          cell.classList.remove('visited');
          cell.classList.add('path');
        }
        // Resolve when last path node is rendered
        if (i === path.length - 1) {
          const doneCleanup = () => {
            gridContainer.classList.remove('animating');
            resolve();
          };
          setTimeout(doneCleanup, PATH_DELAY);
        }
      }, totalVisitTime + PATH_DELAY * i);
    });

    // If no path, still remove animating class after visits
    if (path.length === 0) {
      setTimeout(() => {
        gridContainer.classList.remove('animating');
        resolve();
      }, totalVisitTime);
    }
  });
}
