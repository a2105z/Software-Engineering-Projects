import pygame, sys, random

# === Configuration ===
pygame.init()
FPS = 60
CELL_SIZE    = 36
ROWS, COLS   = 20, 10
X_OFFSET     = 40
Y_OFFSET     = 40
PREVIEW_GAP  = 60
PREVIEW_BOX  = 4
PREVIEW_X    = X_OFFSET + COLS * CELL_SIZE + PREVIEW_GAP
PREVIEW_Y    = Y_OFFSET
# Extend window height to add space for score at bottom
BOTTOM_MARGIN = 80
WINDOW_W     = PREVIEW_X + PREVIEW_BOX * CELL_SIZE + X_OFFSET
WINDOW_H     = Y_OFFSET + ROWS * CELL_SIZE + BOTTOM_MARGIN

SCREEN    = pygame.display.set_mode((WINDOW_W, WINDOW_H))
pygame.display.set_caption("TETRIS")
CLOCK     = pygame.time.Clock()

# === Colors & Fonts ===
BG_COLOR   = pygame.Color("#111111")
GRID_COLOR = pygame.Color("#222222")
COLORS     = [
    GRID_COLOR,            # 0 = empty
    pygame.Color("#47E617"), # 1 = L
    pygame.Color("#E81212"), # 2 = J
    pygame.Color("#17E6E6"), # 3 = I
    pygame.Color("#E26E11"), # 4 = O
    pygame.Color("#EDED04"), # 5 = S
    pygame.Color("#A600F7"), # 6 = T
    pygame.Color("#0D40D8"), # 7 = Z
]
TXT_COLOR  = pygame.Color("#FFFFFF")
FONT_TITLE = pygame.font.Font(None, 48)
FONT_SCORE = pygame.font.Font(None, 32)

# === Helper Classes ===
class Position:
    def __init__(self, r, c):
        self.row, self.col = r, c

# === Block Base ===
class Block:
    def __init__(self, id, rotations):
        self.id = id
        self.rotations = rotations  # list of lists of Position
        self.rot = 0

    def cells(self):
        return self.rotations[self.rot]

    def draw(self, surf, xo, yo):
        for p in self.cells():
            r, c = p.row, p.col
            rect = pygame.Rect(
                xo + c * CELL_SIZE,
                yo + r * CELL_SIZE,
                CELL_SIZE - 1, CELL_SIZE - 1
            )
            pygame.draw.rect(surf, COLORS[self.id], rect)

    def rotate_around_center(self):
        if len(self.rotations) > 1:
            old_center = self.center()
            self.rot = (self.rot + 1) % len(self.rotations)
            new_center = self.center()
            dr = old_center.row - new_center.row
            dc = old_center.col - new_center.col
            for p in self.cells():
                p.row += dr
                p.col += dc

    def undo_rotate(self):
        self.rot = (self.rot - 1) % len(self.rotations)

    def move(self, dr, dc):
        for p in self.cells():
            p.row += dr
            p.col += dc

    def center(self):
        p = self.cells()[1]
        return Position(p.row, p.col)

# === Specific Blocks ===
class IBlock(Block):
    def __init__(self):
        rots = [
            [Position(1,0),Position(1,1),Position(1,2),Position(1,3)],
            [Position(0,2),Position(1,2),Position(2,2),Position(3,2)],
            [Position(2,0),Position(2,1),Position(2,2),Position(2,3)],
            [Position(0,1),Position(1,1),Position(2,1),Position(3,1)]
        ]
        super().__init__(3, rots)
        self.move(-1, 3)
    def center(self):
        base = self.cells()
        if self.rot % 2 == 0:
            return Position(base[1].row, base[2].col)
        else:
            return Position(base[2].row, base[1].col)

class JBlock(Block):
    def __init__(self):
        rots = [
            [Position(0,0),Position(1,0),Position(1,1),Position(1,2)],
            [Position(0,1),Position(0,2),Position(1,1),Position(2,1)],
            [Position(1,0),Position(1,1),Position(1,2),Position(2,2)],
            [Position(0,1),Position(1,1),Position(2,0),Position(2,1)]
        ]
        super().__init__(2, rots)
        self.move(0, 3)

class LBlock(Block):
    def __init__(self):
        rots = [
            [Position(0,2),Position(1,0),Position(1,1),Position(1,2)],
            [Position(0,1),Position(1,1),Position(2,1),Position(2,2)],
            [Position(1,0),Position(1,1),Position(1,2),Position(2,0)],
            [Position(0,0),Position(0,1),Position(1,1),Position(2,1)]
        ]
        super().__init__(1, rots)
        self.move(0, 3)

class OBlock(Block):
    def __init__(self):
        rots = [[Position(0,0),Position(0,1),Position(1,0),Position(1,1)]]
        super().__init__(4, rots)
        self.move(0, 4)

class SBlock(Block):
    def __init__(self):
        rots = [
            [Position(0,1),Position(0,2),Position(1,0),Position(1,1)],
            [Position(0,1),Position(1,1),Position(1,2),Position(2,2)],
            [Position(1,1),Position(1,2),Position(2,0),Position(2,1)],
            [Position(0,0),Position(1,0),Position(1,1),Position(2,1)]
        ]
        super().__init__(5, rots)
        self.move(0, 3)

class TBlock(Block):
    def __init__(self):
        rots = [
            [Position(0,1),Position(1,0),Position(1,1),Position(1,2)],
            [Position(0,1),Position(1,1),Position(1,2),Position(2,1)],
            [Position(1,0),Position(1,1),Position(1,2),Position(2,1)],
            [Position(0,1),Position(1,0),Position(1,1),Position(2,1)]
        ]
        super().__init__(6, rots)
        self.move(0, 3)

class ZBlock(Block):
    def __init__(self):
        rots = [
            [Position(0,0),Position(0,1),Position(1,1),Position(1,2)],
            [Position(0,2),Position(1,1),Position(1,2),Position(2,1)],
            [Position(1,0),Position(1,1),Position(2,1),Position(2,2)],
            [Position(0,1),Position(1,0),Position(1,1),Position(2,0)]
        ]
        super().__init__(7, rots)
        self.move(0, 3)

# === Grid ===
class Grid:
    def __init__(self):
        self.cells = [[0]*COLS for _ in range(ROWS)]
    def inside(self, r, c):
        return 0 <= r < ROWS and 0 <= c < COLS
    def empty(self, r, c):
        return self.cells[r][c] == 0
    def clear_full(self):
        new = [row for row in self.cells if any(v == 0 for v in row)]
        count = ROWS - len(new)
        self.cells = [[0]*COLS for _ in range(count)] + new
        return count
    def draw(self, surf):
        for r in range(ROWS):
            for c in range(COLS):
                rect = pygame.Rect(
                    X_OFFSET + c*CELL_SIZE,
                    Y_OFFSET + r*CELL_SIZE,
                    CELL_SIZE-1, CELL_SIZE-1
                )
                pygame.draw.rect(surf, COLORS[self.cells[r][c]], rect)

# === Game Logic ===
class Game:
    def __init__(self):
        self.reset()

    def reset(self):
        self.grid           = Grid()
        self.bag            = []
        self.current_block  = self.next_block()
        self.next_block_obj = self.next_block()
        self.score          = 0
        self.over           = False

    def next_block(self):
        if not self.bag:
            self.bag = [IBlock(), JBlock(), LBlock(), OBlock(), SBlock(), TBlock(), ZBlock()]
        b = random.choice(self.bag)
        self.bag.remove(b)
        return b

    def spawn(self):
        self.current_block  = self.next_block_obj
        self.next_block_obj = self.next_block()
        if not self.fits(): self.over = True

    def fits(self):
        return all(
            self.grid.inside(p.row, p.col) and self.grid.empty(p.row, p.col)
            for p in self.current_block.cells()
        )

    def lock(self):
        for p in self.current_block.cells():
            self.grid.cells[p.row][p.col] = self.current_block.id
        lines = self.grid.clear_full()
        self.score += [0,100,300,500][lines]
        self.spawn()

    def move_down(self):
        self.current_block.move(1,0)
        if not self.fits():
            self.current_block.move(-1,0)
            self.lock()

    def move_left(self): self.current_block.move(0,-1) or (not self.fits() and self.current_block.move(0,1))
    def move_right(self):self.current_block.move(0,1)  or (not self.fits() and self.current_block.move(0,-1))
    def rotate(self): self.current_block.rotate_around_center() or (not self.fits() and self.current_block.undo_rotate())

    def draw(self):
        SCREEN.fill(BG_COLOR)
        self.grid.draw(SCREEN)
        # grid lines
        for r in range(ROWS+1):
            y = Y_OFFSET + r*CELL_SIZE
            pygame.draw.line(SCREEN, GRID_COLOR, (X_OFFSET,y), (X_OFFSET+COLS*CELL_SIZE,y))
        for c in range(COLS+1):
            x = X_OFFSET + c*CELL_SIZE
            pygame.draw.line(SCREEN, GRID_COLOR, (x,Y_OFFSET), (x,Y_OFFSET+ROWS*CELL_SIZE))
        # blocks
        self.current_block.draw(SCREEN, X_OFFSET, Y_OFFSET)
        # preview
        pygame.draw.rect(SCREEN, GRID_COLOR, (PREVIEW_X,PREVIEW_Y,PREVIEW_BOX*CELL_SIZE,PREVIEW_BOX*CELL_SIZE),2)
        cells = self.next_block_obj.rotations[0]
        cols = [p.col for p in cells]; rows=[p.row for p in cells]
        w,h = (max(cols)-min(cols)+1)*CELL_SIZE, (max(rows)-min(rows)+1)*CELL_SIZE
        offx = PREVIEW_X + (PREVIEW_BOX*CELL_SIZE - w)//2 - min(cols)*CELL_SIZE
        offy = PREVIEW_Y + (PREVIEW_BOX*CELL_SIZE - h)//2 - min(rows)*CELL_SIZE
        self.next_block_obj.draw(SCREEN, offx, offy)
        # score at bottom
        sy = Y_OFFSET + ROWS*CELL_SIZE + 20
        SCREEN.blit(FONT_TITLE.render("Score:", True, TXT_COLOR),(X_OFFSET, sy))
        SCREEN.blit(FONT_SCORE.render(str(self.score), True, TXT_COLOR),(X_OFFSET+120, sy+5))
        if self.over:
            go=FONT_TITLE.render("GAME OVER",True,pygame.Color('red'))
            r=go.get_rect(center=SCREEN.get_rect().center)
            SCREEN.blit(go,r.topleft)
        pygame.display.flip()

# === Input Handling ===
def handle_keys(event, game):
    if event.key==pygame.K_LEFT:  game.move_left()
    if event.key==pygame.K_RIGHT: game.move_right()
    if event.key==pygame.K_DOWN:  game.move_down()
    if event.key==pygame.K_UP:    game.rotate()

# === Main Loop ===
def main():
    game = Game()
    SPAWN = pygame.USEREVENT+1
    pygame.time.set_timer(SPAWN,500)
    while True:
        CLOCK.tick(FPS)
        for e in pygame.event.get():
            if e.type==pygame.QUIT: pygame.quit(); sys.exit()
            if e.type==pygame.KEYDOWN and not game.over: handle_keys(e, game)
            if e.type==SPAWN and not game.over: game.move_down()
        game.draw()

if __name__=="__main__": main()
