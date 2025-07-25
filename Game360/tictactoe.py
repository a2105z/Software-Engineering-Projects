import tkinter as tk
from tkinter import messagebox, simpledialog

class TicTacToeApp(tk.Tk):
    """Tic Tac Toe Deluxe: PvP or PvAI with a dark, professional UI."""
    def __init__(self):
        super().__init__()
        # --- UI Colors ---
        self.bg_color = "#2E2E2E"           # dark background
        self.grid_color = "#4A4A4A"         # cell background
        self.x_color = "#FF6B6B"            # X accent
        self.o_color = "#4ECDC4"            # O accent
        self.button_font = ("Helvetica", 36, "bold")
        self.title_font = ("Helvetica", 32, "bold")
        self.text_font = ("Helvetica", 18)
        
        # --- Window Setup ---
        self.title("Tic Tac Toe Deluxe")
        self.configure(bg=self.bg_color)
        self.resizable(False, False)
        
        # Launch start screen
        self.create_start_screen()

    def create_start_screen(self):
        """Display mode selection: PvP or PvAI."""
        self.start_frame = tk.Frame(self, bg=self.bg_color)
        self.start_frame.pack(padx=50, pady=50)

        tk.Label(
            self.start_frame,
            text="Select Mode",
            bg=self.bg_color,
            fg="#FFFFFF",
            font=self.title_font
        ).pack(pady=(0,30))

        btn_pvp = tk.Button(
            self.start_frame,
            text="Player vs Player",
            bg=self.grid_color,
            fg="#FFFFFF",
            activebackground="#6C6C6C",
            activeforeground="#FFFFFF",
            font=self.text_font,
            command=self.start_pvp,
            relief=tk.FLAT,
            bd=0,
            padx=20, pady=10
        )
        btn_pvp.pack(fill='x', pady=5)

        btn_pvai = tk.Button(
            self.start_frame,
            text="Player vs AI",
            bg=self.grid_color,
            fg="#FFFFFF",
            activebackground="#6C6C6C",
            activeforeground="#FFFFFF",
            font=self.text_font,
            command=self.start_pvai,
            relief=tk.FLAT,
            bd=0,
            padx=20, pady=10
        )
        btn_pvai.pack(fill='x', pady=5)

    def start_pvp(self):
        """Initialize Player vs Player mode."""
        name1 = simpledialog.askstring("Player 1", "Enter name for Player 1:", parent=self) or "Player 1"
        name2 = simpledialog.askstring("Player 2", "Enter name for Player 2:", parent=self) or "Player 2"
        self.setup_game(mode="PVP", names=[name1, name2])

    def start_pvai(self):
        """Initialize Player vs AI mode."""
        name = simpledialog.askstring("Player", "Enter your name:", parent=self) or "Player"
        self.setup_game(mode="PVAI", names=[name, "AI"])

    def setup_game(self, mode, names):
        """Configure the game with selected mode and player names."""
        self.start_frame.destroy()
        self.mode = mode
        self.player_names = names
        self.marks = ["X", "O"]
        self.scores = {names[0]: 0, names[1]: 0}
        self.current = 0           # index of current player
        self.board = [" "]*9     # 3x3 board cells

        # Header frame: scores and turn indicator
        self.header = tk.Frame(self, bg=self.bg_color)
        self.header.pack(fill='x', pady=(10,0))

        self.score_label = tk.Label(
            self.header,
            text=self.format_scores(),
            bg=self.bg_color,
            fg="#FFFFFF",
            font=self.text_font
        )
        self.score_label.pack(side='left', padx=20)

        self.turn_label = tk.Label(
            self.header,
            text=f"Turn: {self.player_names[self.current]}",
            bg=self.bg_color,
            fg="#CCCCCC",
            font=self.text_font
        )
        self.turn_label.pack(side='right', padx=20)

        # Board frame
        self.board_frame = tk.Frame(self, bg=self.bg_color)
        self.board_frame.pack(padx=20, pady=20)
        self.buttons = []
        for idx in range(9):
            btn = tk.Button(
                self.board_frame,
                text=" ",
                bg=self.grid_color,
                fg="#FFFFFF",
                activebackground="#6C6C6C",
                activeforeground="#FFFFFF",
                font=self.button_font,
                command=lambda i=idx: self.on_click(i),
                relief=tk.FLAT,
                bd=2,
                highlightthickness=0,
                width=4,
                height=2
            )
            btn.grid(row=idx//3, column=idx%3, padx=5, pady=5)
            self.buttons.append(btn)

        # If AI starts
        if self.mode == "PVAI" and self.current == 1:
            self.after(500, self.ai_move)

    def format_scores(self):
        """Return string of current scores."""
        p1, p2 = self.player_names
        return f"{p1}: {self.scores[p1]}    {p2}: {self.scores[p2]}"

    def on_click(self, idx):
        """Handle human click on board button."""
        if self.board[idx] != " " or self.is_game_over():
            return
        if self.mode == "PVAI" and self.current == 1:
            return  # ignore clicks during AI turn
        self.make_move(idx)

    def make_move(self, idx):
        """Place mark, update UI, and check game state."""
        mark = self.marks[self.current]
        name = self.player_names[self.current]
        # update model and UI
        self.board[idx] = mark
        color = self.x_color if mark == "X" else self.o_color
        self.buttons[idx].config(text=mark, fg=color)

        # check win
        if self.check_win(mark):
            self.scores[name] += 1
            self.score_label.config(text=self.format_scores())
            self.ask_play_again(f"{name} ({mark}) wins!")
            return
        # check draw
        if " " not in self.board:
            self.ask_play_again("It's a draw!")
            return
        # switch turn
        self.current ^= 1
        next_name = self.player_names[self.current]
        self.turn_label.config(text=f"Turn: {next_name}")
        # if AI turn
        if self.mode == "PVAI" and self.current == 1:
            self.after(500, self.ai_move)

    def ai_move(self):
        """Compute and execute the AI's move via minimax."""
        best_score = -float('inf')
        best_move = None
        for i, cell in enumerate(self.board):
            if cell == " ":
                self.board[i] = self.marks[1]
                score = self.minimax(False)
                self.board[i] = " "
                if score > best_score:
                    best_score = score
                    best_move = i
        if best_move is not None:
            self.make_move(best_move)

    def minimax(self, is_maximizing):
        """Minimax algorithm for AI decision-making."""
        # base cases
        if self.check_win(self.marks[1]):
            return 1
        if self.check_win(self.marks[0]):
            return -1
        if " " not in self.board:
            return 0
        # recursion
        if is_maximizing:
            best = -float('inf')
            for i in range(9):
                if self.board[i] == " ":
                    self.board[i] = self.marks[1]
                    val = self.minimax(False)
                    self.board[i] = " "
                    best = max(best, val)
            return best
        else:
            best = float('inf')
            for i in range(9):
                if self.board[i] == " ":
                    self.board[i] = self.marks[0]
                    val = self.minimax(True)
                    self.board[i] = " "
                    best = min(best, val)
            return best

    def check_win(self, mark):
        """Check if the given mark has a winning combination."""
        wins = [
            (0,1,2),(3,4,5),(6,7,8),
            (0,3,6),(1,4,7),(2,5,8),
            (0,4,8),(2,4,6)
        ]
        return any(self.board[a]==self.board[b]==self.board[c]==mark for a,b,c in wins)

    def is_game_over(self):
        """Return True if there's a win or a full board."""
        return (self.check_win(self.marks[0]) or
                self.check_win(self.marks[1]) or
                " " not in self.board)

    def ask_play_again(self, message):
        """Prompt to play again or exit after game over."""
        if messagebox.askyesno("Game Over", f"{message}\nPlay again?", parent=self):
            self.reset_board()
        else:
            self.destroy()

    def reset_board(self):
        """Clear board and reset for next round."""
        self.board = [" "]*9
        for btn in self.buttons:
            btn.config(text=" ", fg="#FFFFFF")
        self.current = 0
        self.turn_label.config(text=f"Turn: {self.player_names[self.current]}")
        # if AI starts
        if self.mode == "PVAI" and self.current == 1:
            self.after(500, self.ai_move)

if __name__ == "__main__":
    app = TicTacToeApp()
    app.mainloop()
