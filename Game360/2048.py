from tkinter import Frame, Label, CENTER
import numpy as np
import random

# Constants for the game layout and behavior
EDGE_LENGTH = 400
CELL_COUNT = 4
CELL_PAD = 10
LABEL_FONT = ("Verdana", 40, "bold")
GAME_COLOR = "#a6bdbb"
EMPTY_COLOR = "#8eaba8"

# Color maps for tiles
TILE_COLORS = {
    2: "#daeddf", 4: "#9ae3ae", 8: "#6ce68d", 16: "#42ed71",
    32: "#17e650", 64: "#17c246", 128: "#149938",
    256: "#107d2e", 512: "#0e6325", 1024: "#0b4a1c",
    2048: "#031f0a", 4096: "#000000", 8192: "#000000",
}
LABEL_COLORS = {val: ("#011c08" if val < 64 else "#f2f2f0") for val in TILE_COLORS}

# Probability distribution for new tiles (90% 2, 10% 4)
NEW_TILE_DISTRIBUTION = [2] * 9 + [4]

# Game logic functions
def initialize_game():
    """
    Create a new 4x4 board with two '2' tiles placed at random positions.
    """
    board = np.zeros((CELL_COUNT, CELL_COUNT), dtype=int)
    # Randomly choose two distinct cells to start with '2'
    cells = np.random.choice(CELL_COUNT * CELL_COUNT, 2, replace=False)
    for c in cells:
        row, col = divmod(c, CELL_COUNT)
        board[row, col] = 2
    return board


def push_board_right(board):
    """ Slide all non-zero tiles to the right, preserving order. Returns the new board and a flag indicating if any tile moved. """
    new = np.zeros_like(board)
    moved = False
    for r in range(CELL_COUNT):
        pos = CELL_COUNT - 1
        for c in range(CELL_COUNT - 1, -1, -1):
            if board[r, c] != 0:
                new[r, pos] = board[r, c]
                if c != pos:
                    moved = True
                pos -= 1
    return new, moved


def merge_tiles(board):
    """ Merge equal adjacent tiles in each row (right-to-left). Returns updated board,merge flag, and score gained this merge. """
    score = 0
    merged_any = False
    for r in range(CELL_COUNT):
        for c in range(CELL_COUNT - 1, 0, -1):
            if board[r, c] == board[r, c - 1] != 0:
                board[r, c] *= 2
                board[r, c - 1] = 0
                score += board[r, c]
                merged_any = True
    return board, merged_any, score


def move_right(board):
    """ Execute a right move: push, merge, then push again. Returns new board, move flag, and score gained. """
    pushed, moved1 = push_board_right(board)
    merged, merged_flag, score = merge_tiles(pushed)
    pushed2, _ = push_board_right(merged)
    return pushed2, (moved1 or merged_flag), score


def move_left(board):
    """ Execute a left move by rotating twice, performing a right move, then rotating back. """
    rotated = np.rot90(board, 2)
    moved_board, moved, score = move_right(rotated)
    return np.rot90(moved_board, 2), moved, score


def move_up(board):
    """ Execute an up move by rotating -90°, right move, then rotating back. """
    rotated = np.rot90(board, -1)
    moved_board, moved, score = move_right(rotated)
    return np.rot90(moved_board, 1), moved, score


def move_down(board):
    """ Execute a down move by rotating 90°, right move, then rotating back. """
    rotated = np.rot90(board, 1)
    moved_board, moved, score = move_right(rotated)
    return np.rot90(moved_board, -1), moved, score


def add_new_tile(board):
    """ Add a new tile (2 or 4) into a random empty cell. """
    empties = list(zip(*np.where(board == 0)))
    if not empties:
        return board
    r, c = random.choice(empties)
    board[r, c] = random.choice(NEW_TILE_DISTRIBUTION)
    return board


def check_game_over(board):
    """ Check if no moves are possible (no empties and no adjacent equals). """
    if np.any(board == 0):
        return False
    # Check horizontal and vertical possible merges
    for r in range(CELL_COUNT):
        for c in range(CELL_COUNT - 1):
            if board[r, c] == board[r, c + 1] or board[c, r] == board[c + 1, r]:
                return False
    return True


class Display(Frame):
    """ Main 2048 game display. Handles rendering, user input, and score tracking. """
    def __init__(self):
        """ Set up the grid, score display, and key bindings. """
        super().__init__()
        self.grid()
        self.master.title('2048')

        # Track the game state and score
        self.matrix = initialize_game()
        self.score = 0

        # Score label above the grid
        self.score_label = Label(self, text=f"Score: {self.score}", font=("Verdana", 20, "bold"))
        self.score_label.grid(row=0, column=0, pady=(10, 0))

        # Build the visual grid below the score
        self.grid_cells = []
        self.build_grid()

        # Bind arrow keys for control
        self.master.bind("<Up>", self.key_press)
        self.master.bind("<Down>", self.key_press)
        self.master.bind("<Left>", self.key_press)
        self.master.bind("<Right>", self.key_press)

        # Draw initial board
        self.draw_grid_cells()
        self.mainloop()

    def build_grid(self):
        """ Create the 4x4 grid of Labels within colored Frames. """
        background = Frame(self, bg=GAME_COLOR, width=EDGE_LENGTH, height=EDGE_LENGTH)
        background.grid(row=1, column=0)
        for r in range(CELL_COUNT):
            row_cells = []
            for c in range(CELL_COUNT):
                cell = Frame(
                    background,
                    bg=EMPTY_COLOR,
                    width=EDGE_LENGTH / CELL_COUNT,
                    height=EDGE_LENGTH / CELL_COUNT
                )
                cell.grid(row=r, column=c, padx=CELL_PAD, pady=CELL_PAD)
                label = Label(
                    master=cell,
                    text="",
                    bg=EMPTY_COLOR,
                    justify=CENTER,
                    font=LABEL_FONT,
                    width=5,
                    height=2
                )
                label.grid()
                row_cells.append(label)
            self.grid_cells.append(row_cells)

    def draw_grid_cells(self):
        """ Update each cell's text and background color based on the matrix. """
        for r in range(CELL_COUNT):
            for c in range(CELL_COUNT):
                val = self.matrix[r, c]
                cell = self.grid_cells[r][c]
                if val == 0:
                    cell.configure(text="", bg=EMPTY_COLOR)
                else:
                    cell.configure(
                        text=str(val),
                        bg=TILE_COLORS.get(val, "#000000"),
                        fg=LABEL_COLORS.get(val, "#f2f2f0")
                    )
        self.update_idletasks()

    def key_press(self, event):
        """ Handle arrow key presses: move tiles, add a new tile, update score, and check for game over. """
        move_map = {'Up': move_up, 'Down': move_down, 'Left': move_left, 'Right': move_right }
        key = event.keysym
        if key in move_map:
            # Perform the move
            new_board, moved, gained = move_map[key](self.matrix)
            if moved:
                self.matrix = add_new_tile(new_board)
                self.score += gained
                self.score_label.configure(text=f"Score: {self.score}")
                self.draw_grid_cells()

                if np.any(self.matrix == 2048):
                    self.score_label.configure(text="🎉 You Win! 🎉")
                    self.master.unbind("<Up>")
                    self.master.unbind("<Down>")
                    self.master.unbind("<Left>")
                    self.master.unbind("<Right>")
                    return

                # If no more moves, display game over
                if check_game_over(self.matrix):
                    self.score_label.configure(text=f"Game Over! Final Score: {self.score}")


if __name__ == "__main__":
    # Launch the game
    Display()