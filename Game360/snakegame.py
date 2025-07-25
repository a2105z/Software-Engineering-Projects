from tkinter import *
import random

# === Constants ===
GAME_WIDTH = 800
GAME_HEIGHT = 600
SPACE_SIZE = 30
INITIAL_SNAKE_LENGTH = 3
BASE_SPEED = 150        # initial delay in ms
SPEED_INCREMENT = -5    # speed up per point
MIN_SPEED = 50          # minimum delay

# Colors
BG_COLOR = "#111111"
GRID_COLOR = "#222222"
BORDER_COLOR = "#555555"
HEAD_COLOR = "#00FF00"
TAIL_COLOR = "#008000"
FOOD_COLORS = ["#FF0000", "#FFFF00", "#FF00FF", "#00FFFF", "#FFFFFF"]

class Game:
    """Enhanced Snake game with improved UI and dynamic speed."""
    def __init__(self, root):
        self.root = root
        self.state = 'START'  # START, RUNNING, GAMEOVER
        self.score = 0
        self.high_score = 0
        self.direction = 'Right'
        self.snake_coords = []
        self.food_coord = ()
        
        # Set up UI
        root.title("Snake")
        self.label = Label(root, text="Press Enter to Start", font=('Consolas', 24), fg='white', bg=BG_COLOR)
        self.label.pack(fill=X)
        self.canvas = Canvas(root, bg=BG_COLOR, width=GAME_WIDTH, height=GAME_HEIGHT, highlightthickness=0)
        self.canvas.pack()
        
        # Bindings
        root.bind('<Return>', self.on_enter)
        root.bind('<Up>', lambda e: self.change_direction('Up'))
        root.bind('<Down>', lambda e: self.change_direction('Down'))
        root.bind('<Left>', lambda e: self.change_direction('Left'))
        root.bind('<Right>', lambda e: self.change_direction('Right'))
        
        self.draw_start()

    def draw_start(self):
        """Display start screen."""
        self.canvas.delete('all')
        self.canvas.create_text(GAME_WIDTH//2, GAME_HEIGHT//2,
                                text="Press Enter to Start",
                                fill="white", font=('Consolas', 32))

    def on_enter(self, event):
        if self.state in ('START', 'GAMEOVER'):
            self.reset()

    def reset(self):
        """Initialize or restart the game."""
        self.state = 'RUNNING'
        self.score = 0
        self.direction = 'Right'
        # Initialize snake in center
        start_x = GAME_WIDTH//2 // SPACE_SIZE * SPACE_SIZE
        start_y = GAME_HEIGHT//2 // SPACE_SIZE * SPACE_SIZE
        self.snake_coords = [(start_x - i*SPACE_SIZE, start_y) for i in range(INITIAL_SNAKE_LENGTH)]
        # Place first food
        self.place_food()
        self.label.config(text=f"Score: {self.score}   High Score: {self.high_score}")
        self.move_snake()

    def place_food(self):
        """Place food at a random empty cell."""
        max_x = GAME_WIDTH // SPACE_SIZE
        max_y = GAME_HEIGHT // SPACE_SIZE
        while True:
            fx = random.randint(0, max_x - 1) * SPACE_SIZE
            fy = random.randint(0, max_y - 1) * SPACE_SIZE
            if (fx, fy) not in self.snake_coords:
                self.food_coord = (fx, fy)
                break

    def change_direction(self, new_dir):
        """Change snake direction, preventing direct reversal."""
        opposites = {'Up':'Down','Down':'Up','Left':'Right','Right':'Left'}
        if self.state == 'RUNNING' and new_dir != opposites.get(self.direction):
            self.direction = new_dir

    def move_snake(self):
        if self.state != 'RUNNING':
            return
        
        # Dynamic speed based on score
        interval = max(MIN_SPEED, BASE_SPEED + self.score * SPEED_INCREMENT)
        
        # Compute new head
        head_x, head_y = self.snake_coords[0]
        if self.direction == 'Up':
            head_y -= SPACE_SIZE
        elif self.direction == 'Down':
            head_y += SPACE_SIZE
        elif self.direction == 'Left':
            head_x -= SPACE_SIZE
        elif self.direction == 'Right':
            head_x += SPACE_SIZE
        
        new_head = (head_x, head_y)
        self.snake_coords.insert(0, new_head)
        
        # Check food collision
        if new_head == self.food_coord:
            self.score += 1
            if self.score > self.high_score:
                self.high_score = self.score
            self.place_food()
            self.label.config(text=f"Score: {self.score}   High Score: {self.high_score}")
        else:
            self.snake_coords.pop()
        
        # Check collisions
        if (head_x < 0 or head_x >= GAME_WIDTH or head_y < 0 or head_y >= GAME_HEIGHT
            or new_head in self.snake_coords[1:]):
            self.game_over()
            return
        
        # Redraw everything
        self.draw_scene()
        self.root.after(interval, self.move_snake)

    def draw_scene(self):
        """Draw grid, border, snake, and food."""
        self.canvas.delete('all')
        # Border
        self.canvas.create_rectangle(0, 0, GAME_WIDTH, GAME_HEIGHT, outline=BORDER_COLOR)
        # Grid lines
        for x in range(0, GAME_WIDTH, SPACE_SIZE):
            self.canvas.create_line(x, 0, x, GAME_HEIGHT, fill=GRID_COLOR)
        for y in range(0, GAME_HEIGHT, SPACE_SIZE):
            self.canvas.create_line(0, y, GAME_WIDTH, y, fill=GRID_COLOR)
        # Draw food
        fx, fy = self.food_coord
        color = random.choice(FOOD_COLORS)
        self.canvas.create_oval(fx, fy, fx+SPACE_SIZE, fy+SPACE_SIZE, fill=color, outline='')
        # Draw snake segments
        for i, (x, y) in enumerate(self.snake_coords):
            # Gradient from head to tail
            t = i / len(self.snake_coords)
            r = int(HEAD_COLOR[1:3],16)*(1-t) + int(TAIL_COLOR[1:3],16)*t  # placeholder gradient calc
            fill = HEAD_COLOR if i == 0 else TAIL_COLOR
            self.canvas.create_rectangle(x, y, x+SPACE_SIZE, y+SPACE_SIZE,
                                         fill=fill, outline='')
    
    def game_over(self):
        """Handle game over state."""
        self.state = 'GAMEOVER'
        self.canvas.create_text(GAME_WIDTH//2, GAME_HEIGHT//2,
                                text="GAME OVER\nPress Enter to Restart",
                                fill="red", font=('Consolas', 32))

if __name__ == "__main__":
    root = Tk()
    root.configure(bg=BG_COLOR)
    root.resizable(False, False)
    game = Game(root)
    root.mainloop()