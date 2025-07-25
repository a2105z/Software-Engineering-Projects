import pygame
import sys

# ──────────────────────────────────────────────────────────────────────────────
# Constants
# ──────────────────────────────────────────────────────────────────────────────
BOARD_SIZE       = 8
SQUARE_SIZE      = 100
BOARD_PIXELS     = BOARD_SIZE * SQUARE_SIZE      # 800
UI_PANEL_WIDTH   = 200
UI_PANEL_HEIGHT  = 100
WIDTH            = BOARD_PIXELS + UI_PANEL_WIDTH  # 1000
HEIGHT           = BOARD_PIXELS + UI_PANEL_HEIGHT # 900

# Colors
WHITE     = (255, 255, 255)
BLACK     = (0,   0,   0)
LIGHT_GREY= (200, 200, 200)
DARK_GREY = ( 64,  64,  64)
UI_GREY   = (128, 128, 128)
GOLD      = (212, 175,  55)
RED       = (255,   0,   0)
BLUE      = (  0,   0, 255)

# Map piece names to single‑letter codes (knight → N)
LETTER_MAP = {
    'pawn':   'P',
    'rook':   'R',
    'knight': 'N',
    'bishop': 'B',
    'queen':  'Q',
    'king':   'K'
}

FPS = 60

# ──────────────────────────────────────────────────────────────────────────────
# Initialization
# ──────────────────────────────────────────────────────────────────────────────
def init_pygame():
    pygame.init()
    screen = pygame.display.set_mode((WIDTH, HEIGHT))
    pygame.display.set_caption('Two-Player Pygame Chess!')
    return screen

screen       = init_pygame()
font         = pygame.font.Font('freesansbold.ttf', 20)
medium_font  = pygame.font.Font('freesansbold.ttf', 40)
big_font     = pygame.font.Font('freesansbold.ttf', 50)
clock        = pygame.time.Clock()

# ──────────────────────────────────────────────────────────────────────────────
# Game State Globals
# ──────────────────────────────────────────────────────────────────────────────
white_pieces     = []
white_locations  = []
black_pieces     = []
black_locations  = []
captured_white   = []
captured_black   = []
white_options    = []
black_options    = []
turn_step        = 0    # 0: W select, 1: W move, 2: B select, 3: B move
selection        = None
valid_moves      = []
winner           = ''
game_over        = False
flash_counter    = 0

# ──────────────────────────────────────────────────────────────────────────────
# Move‑Generation Logic
# ──────────────────────────────────────────────────────────────────────────────
def check_pawn(pos, color):
    x, y = pos
    moves = []
    if color == 'white':
        if y < 7 and (x, y+1) not in white_locations and (x, y+1) not in black_locations:
            moves.append((x, y+1))
            if y == 1 and (x, y+2) not in white_locations and (x, y+2) not in black_locations:
                moves.append((x, y+2))
        for dx in (-1, 1):
            nx, ny = x+dx, y+1
            if 0 <= nx < 8 and (nx, ny) in black_locations:
                moves.append((nx, ny))
    else:
        if y > 0 and (x, y-1) not in white_locations and (x, y-1) not in black_locations:
            moves.append((x, y-1))
            if y == 6 and (x, y-2) not in white_locations and (x, y-2) not in black_locations:
                moves.append((x, y-2))
        for dx in (-1, 1):
            nx, ny = x+dx, y-1
            if 0 <= nx < 8 and (nx, ny) in white_locations:
                moves.append((nx, ny))
    return moves

def check_knight(pos, color):
    x, y = pos
    friends = white_locations if color=='white' else black_locations
    moves = []
    for dx, dy in [(2,1),(2,-1),(-2,1),(-2,-1),(1,2),(1,-2),(-1,2),(-1,-2)]:
        nx, ny = x+dx, y+dy
        if 0 <= nx < 8 and 0 <= ny < 8 and (nx, ny) not in friends:
            moves.append((nx, ny))
    return moves

def check_sliding(pos, color, directions):
    friends = white_locations if color=='white' else black_locations
    enemies = black_locations if color=='white' else white_locations
    moves = []
    x0, y0 = pos
    for dx, dy in directions:
        dist = 1
        while True:
            x, y = x0 + dx*dist, y0 + dy*dist
            if not (0 <= x < 8 and 0 <= y < 8): break
            if (x,y) in friends: break
            moves.append((x,y))
            if (x,y) in enemies: break
            dist += 1
    return moves

def check_rook(pos, color):    return check_sliding(pos, color, [(1,0),(-1,0),(0,1),(0,-1)])
def check_bishop(pos, color):  return check_sliding(pos, color, [(1,1),(1,-1),(-1,1),(-1,-1)])
def check_queen(pos, color):   return check_rook(pos, color) + check_bishop(pos, color)

def check_king(pos, color):
    friends = white_locations if color=='white' else black_locations
    moves = []
    for dx, dy in [(1,0),(1,1),(1,-1),(-1,0),(-1,1),(-1,-1),(0,1),(0,-1)]:
        x, y = pos[0]+dx, pos[1]+dy
        if 0 <= x < 8 and 0 <= y < 8 and (x,y) not in friends:
            moves.append((x,y))
    return moves

def check_options(pieces, locations, color):
    all_moves = []
    for i, p in enumerate(pieces):
        pos = locations[i]
        if   p=='pawn':   m = check_pawn(pos,color)
        elif p=='knight': m = check_knight(pos,color)
        elif p=='rook':   m = check_rook(pos,color)
        elif p=='bishop': m = check_bishop(pos,color)
        elif p=='queen':  m = check_queen(pos,color)
        elif p=='king':   m = check_king(pos,color)
        else:             m = []
        all_moves.append(m)
    return all_moves

# ──────────────────────────────────────────────────────────────────────────────
# Game Initialization / Reset
# ──────────────────────────────────────────────────────────────────────────────
def init_game_state():
    global white_pieces, white_locations, black_pieces, black_locations
    global captured_white, captured_black, white_options, black_options
    global turn_step, selection, valid_moves, winner, game_over, flash_counter

    white_pieces    = ['rook','knight','bishop','king','queen','bishop','knight','rook'] + ['pawn']*8
    white_locations = [(x,0) for x in range(8)] + [(x,1) for x in range(8)]
    black_pieces    = ['rook','knight','bishop','king','queen','bishop','knight','rook'] + ['pawn']*8
    black_locations = [(x,7) for x in range(8)] + [(x,6) for x in range(8)]

    captured_white  = []
    captured_black  = []

    turn_step       = 0
    selection       = None
    valid_moves     = []
    winner          = ''
    game_over       = False
    flash_counter   = 0

    white_options   = check_options(white_pieces, white_locations, 'white')
    black_options   = check_options(black_pieces, black_locations, 'black')

# ──────────────────────────────────────────────────────────────────────────────
# Drawing Helpers
# ──────────────────────────────────────────────────────────────────────────────
def draw_board():
    for r in range(8):
        for c in range(8):
            col = LIGHT_GREY if (r+c)%2==0 else DARK_GREY
            pygame.draw.rect(screen, col, (c*SQUARE_SIZE, r*SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE))
    for i in range(9):
        pygame.draw.line(screen, BLACK, (0, i*SQUARE_SIZE), (BOARD_PIXELS, i*SQUARE_SIZE), 2)
        pygame.draw.line(screen, BLACK, (i*SQUARE_SIZE, 0), (i*SQUARE_SIZE, BOARD_PIXELS), 2)
    pygame.draw.rect(screen, UI_GREY, (0, BOARD_PIXELS, WIDTH, UI_PANEL_HEIGHT))
    pygame.draw.rect(screen, GOLD,    (0, BOARD_PIXELS, WIDTH, UI_PANEL_HEIGHT), 5)
    pygame.draw.rect(screen, GOLD,    (BOARD_PIXELS, 0, UI_PANEL_WIDTH, BOARD_PIXELS), 5)
    status = ['White: Select Piece', 'White: Select Destination',
              'Black: Select Piece', 'Black: Select Destination']
    txt = big_font.render(status[turn_step], True, BLACK)
    screen.blit(txt, (10, BOARD_PIXELS + 10))
    ftxt = medium_font.render('FORFEIT', True, BLACK)
    screen.blit(ftxt, (BOARD_PIXELS + 10, BOARD_PIXELS + 20))

def draw_piece(color, letter, sq):
    cx = sq[0]*SQUARE_SIZE + SQUARE_SIZE//2
    cy = sq[1]*SQUARE_SIZE + SQUARE_SIZE//2
    pygame.draw.circle(screen, color, (cx, cy), SQUARE_SIZE//2 - 10)
    text_col = WHITE if color==BLACK else BLACK
    txt = font.render(letter, True, text_col)
    screen.blit(txt, txt.get_rect(center=(cx, cy)))

def draw_pieces():
    for i, p in enumerate(white_pieces):
        draw_piece(WHITE, LETTER_MAP[p], white_locations[i])
        if turn_step < 2 and selection==i:
            x,y = white_locations[i]
            pygame.draw.rect(screen, RED, (x*SQUARE_SIZE+1, y*SQUARE_SIZE+1, SQUARE_SIZE-2, SQUARE_SIZE-2), 3)
    for i, p in enumerate(black_pieces):
        draw_piece(BLACK, LETTER_MAP[p], black_locations[i])
        if turn_step >= 2 and selection==i:
            x,y = black_locations[i]
            pygame.draw.rect(screen, BLUE, (x*SQUARE_SIZE+1, y*SQUARE_SIZE+1, SQUARE_SIZE-2, SQUARE_SIZE-2), 3)

def draw_valid_moves():
    col = RED if turn_step<2 else BLUE
    for x,y in valid_moves:
        cx = x*SQUARE_SIZE + SQUARE_SIZE//2
        cy = y*SQUARE_SIZE + SQUARE_SIZE//2
        pygame.draw.circle(screen, col, (cx, cy), 6)

def draw_captured():
    for i, p in enumerate(captured_white):
        letter = LETTER_MAP[p]
        cx = BOARD_PIXELS + 25
        cy = 25 + i*40
        pygame.draw.circle(screen, BLACK, (cx, cy), 20)
        txt = font.render(letter, True, WHITE)
        screen.blit(txt, txt.get_rect(center=(cx, cy)))
    for i, p in enumerate(captured_black):
        letter = LETTER_MAP[p]
        cx = BOARD_PIXELS + 125
        cy = 25 + i*40
        pygame.draw.circle(screen, WHITE, (cx, cy), 20)
        txt = font.render(letter, True, BLACK)
        screen.blit(txt, txt.get_rect(center=(cx, cy)))

def draw_check_flash():
    global flash_counter
    flash_counter = (flash_counter + 1) % 30
    if flash_counter < 15:
        if turn_step < 2 and 'king' in white_pieces:
            idx = white_pieces.index('king')
            kpos = white_locations[idx]
            for moves in black_options:
                if kpos in moves:
                    pygame.draw.rect(screen, RED, (kpos[0]*SQUARE_SIZE+1, kpos[1]*SQUARE_SIZE+1, SQUARE_SIZE-2, SQUARE_SIZE-2), 4)
                    break
        if turn_step >= 2 and 'king' in black_pieces:
            idx = black_pieces.index('king')
            kpos = black_locations[idx]
            for moves in white_options:
                if kpos in moves:
                    pygame.draw.rect(screen, BLUE, (kpos[0]*SQUARE_SIZE+1, kpos[1]*SQUARE_SIZE+1, SQUARE_SIZE-2, SQUARE_SIZE-2), 4)
                    break

def draw_game_over():
    rect = pygame.Rect(BOARD_PIXELS//2-200, BOARD_PIXELS//2-50, 400, 100)
    pygame.draw.rect(screen, BLACK, rect)
    wm = font.render(f'{winner} wins!', True, WHITE)
    screen.blit(wm, wm.get_rect(center=(BOARD_PIXELS//2, BOARD_PIXELS//2-10)))
    rm = font.render('Press ENTER to restart', True, WHITE)
    screen.blit(rm, rm.get_rect(center=(BOARD_PIXELS//2, BOARD_PIXELS//2+20)))

# ──────────────────────────────────────────────────────────────────────────────
# Start / Main Loop
# ──────────────────────────────────────────────────────────────────────────────
init_game_state()

running = True
while running:
    clock.tick(FPS)
    screen.fill(DARK_GREY)

    draw_board()
    draw_pieces()
    draw_captured()
    draw_check_flash()
    if selection is not None:
        valid_moves = (white_options if turn_step<2 else black_options)[selection]
        draw_valid_moves()
    if game_over:
        draw_game_over()

    for e in pygame.event.get():
        if e.type == pygame.QUIT:
            running = False

        # Deselect on ESC
        elif e.type == pygame.KEYDOWN and e.key == pygame.K_ESCAPE and not game_over and selection is not None:
            if turn_step == 1:
                turn_step = 0
            elif turn_step == 3:
                turn_step = 2
            selection = None
            valid_moves = []

        # Mouse click handling
        elif e.type == pygame.MOUSEBUTTONDOWN and e.button == 1 and not game_over:
            mx, my = e.pos
            bx, by = mx // SQUARE_SIZE, my // SQUARE_SIZE

            # Deselect by clicking the same square
            if selection is not None:
                if turn_step < 2 and (bx,by) == white_locations[selection]:
                    turn_step = 0
                    selection = None
                    valid_moves = []
                    continue
                if turn_step >= 2 and (bx,by) == black_locations[selection]:
                    turn_step = 2
                    selection = None
                    valid_moves = []
                    continue

            # Forfeit
            if bx >= 8 and by >= 8:
                winner = 'Black' if turn_step < 2 else 'White'
                game_over = True
                continue

            # White's turn
            if turn_step < 2:
                if selection is None and (bx,by) in white_locations:
                    selection = white_locations.index((bx,by))
                    turn_step = 1
                elif selection is not None and (bx,by) in valid_moves:
                    if (bx,by) in black_locations:
                        cap = black_locations.index((bx,by))
                        captured_white.append(black_pieces[cap])
                        if black_pieces[cap]=='king':
                            winner = 'White'; game_over = True
                        del black_pieces[cap], black_locations[cap]
                    white_locations[selection] = (bx,by)
                    white_options = check_options(white_pieces, white_locations, 'white')
                    black_options = check_options(black_pieces, black_locations, 'black')
                    turn_step = 2
                    selection = None

            # Black's turn
            else:
                if selection is None and (bx,by) in black_locations:
                    selection = black_locations.index((bx,by))
                    turn_step = 3
                elif selection is not None and (bx,by) in valid_moves:
                    if (bx,by) in white_locations:
                        cap = white_locations.index((bx,by))
                        captured_black.append(white_pieces[cap])
                        if white_pieces[cap]=='king':
                            winner = 'Black'; game_over = True
                        del white_pieces[cap], white_locations[cap]
                    black_locations[selection] = (bx,by)
                    white_options = check_options(white_pieces, white_locations, 'white')
                    black_options = check_options(black_pieces, black_locations, 'black')
                    turn_step = 0
                    selection = None

        # Restart on Enter
        elif e.type == pygame.KEYDOWN and game_over and e.key == pygame.K_RETURN:
            init_game_state()

    pygame.display.flip()

pygame.quit()
sys.exit()
