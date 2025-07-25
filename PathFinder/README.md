# PathFinder – Algorithm Visualization Web App

PathFinder is an interactive web app that allows users to visualize how various pathfinding algorithms explore a grid to find the shortest path between two points.

---

## 🔍 Features

- Interactive grid: click and drag to place **walls**, **start**, and **end** nodes
- Visualize step-by-step how each algorithm traverses the space
- Supports multiple pathfinding algorithms:
  - ✅ Breadth-First Search (BFS)
  - ✅ Depth-First Search (DFS)
  - ✅ Dijkstra's Algorithm
  - ✅ A* Search
  - ✅ Greedy Best-First Search (GBFS)
  - ✅ Bidirectional Search
  - ✅ Jump Point Search (JPS)
  - ✅ Recursive Best-First Search (RBFS)
- Beautiful, color-coded animations
- Fully modular front-end and back-end architecture

---

## 🧱 Tech Stack

- **Frontend**: HTML, CSS (custom theme), JavaScript (ES modules)
- **Backend**: Python 3 + Flask
- **Visualization Logic**: Algorithms written in Python and served over a REST API
- **Localhost Hosting**: Runs entirely locally—no database or external APIs

---


## 🚀 Getting Started


### 1. Clone the repository

git clone https://github.com/yourusername/pathfinder-visualizer.git
cd pathfinder-visualizer

### 2. Start the Python Flask back end
cd backend
python -m venv venv         # Optional but recommended
source venv/bin/activate    # On Windows: .\venv\Scripts\activate (if you use vnev)
pip install -r requirements.txt
python app.py

Flask should start on:
http://localhost:5000


### 3. Start the front-end static server
In a second terminal:
    cd frontend
    python -m http.server 8000

This serves the front end on:
http://localhost:8000

### 4. Open the app
Go to http://localhost:8000 in your browser.
    Left-click to place start, end, and walls
    Right-click to erase
    Select an algorithm
    Press Run to visualize
    Press Clear to reset the grid







