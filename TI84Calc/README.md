# GraphForge

A modern, TI-84 inspired graphing calculator — clean like Desmos, with the feel of a classic calculator. Built as a web app.

![GraphForge](https://img.shields.io/badge/GraphForge-Graphing%20Calculator-00C853)

## Features

- **Dual mode**: Scientific calculator + graphing
- **Multiple functions**: Graph up to 4 functions (y₁, y₂, y₃, y₄) simultaneously
- **Desmos-like interactions**: Pan (click-drag) and zoom (scroll wheel)
- **Trace mode**: Hover over the graph to see coordinates
- **Window settings**: Customize X and Y ranges
- **Scientific functions**: sin, cos, tan, log, ln, sqrt, exp, abs
- **Angle modes**: Switch between RAD and DEG
- **TI-84 aesthetic**: Dark gray keypad, clean display

## Try it

**Live on GitHub Pages:** After pushing and enabling Pages, your site will be at  
`https://<username>.github.io/<repo-name>/`

## Run locally

1. Clone the repo
2. Open `docs/index.html` in a browser, or serve the `docs` folder:

```bash
# Using Python
cd docs && python -m http.server 8000

# Using Node (npx serve)
npx serve docs
```

## Deploy to GitHub Pages

**Option A — GitHub Actions (recommended for this repo structure):**

1. Push to GitHub
2. Go to **Settings → Pages**
3. Under "Build and deployment", set **Source** to **"GitHub Actions"**
4. The workflow deploys on push to `main` when `TI84Calc/docs/` changes

**Option B — Deploy from branch (if you move docs to repo root):**

1. Move `docs/` to the repo root
2. Set **Source** to "Deploy from a branch"
3. Select branch `main` and folder **`/docs`**

## Syntax

| Input | Meaning |
|-------|---------|
| `^` or `**` | Power (e.g. `x^2`) |
| `sin`, `cos`, `tan` | Trig (respect RAD/DEG) |
| `log` | Base-10 logarithm |
| `ln` | Natural logarithm |
| `sqrt`, `abs`, `exp` | Standard functions |
| `pi`, `e` | Constants |

## Name

**GraphForge** — Forging graphs, one curve at a time. A unique name that suggests creation and precision.
