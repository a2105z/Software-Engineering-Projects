import pygame, sys, random

# Initialize Pygame
pygame.init()

# Screen dimensions and setup
WIDTH, HEIGHT = 900, 500
SCREEN = pygame.display.set_mode((WIDTH, HEIGHT))
pygame.display.set_caption("Pong Deluxe")
CLOCK = pygame.time.Clock()
FPS = 60

# Game states
STATE_MENU   = "MENU"
STATE_NAME1  = "NAME1"
STATE_NAME2  = "NAME2"
STATE_PLAY   = "PLAY"

# Colors
COLOR_BG     = pygame.Color('#0b2545')
COLOR_P1     = pygame.Color('#e63946')
COLOR_P2     = pygame.Color('#2a9d8f')
COLOR_BALL   = pygame.Color('#f4a261')
COLOR_TEXT   = pygame.Color('#ffffff')
COLOR_BUTTON = pygame.Color('#264653')

# Fonts
FONT_TITLE  = pygame.font.SysFont('Consolas', 64, bold=True)
FONT_BUTTON = pygame.font.SysFont('Consolas', 32)
FONT_SCORE  = pygame.font.SysFont('Consolas', 48)
FONT_NAME   = pygame.font.SysFont('Consolas', 28)

# Gameplay constants
PADDLE_WIDTH, PADDLE_HEIGHT = 20, 120
BALL_SIZE = 20
PADDLE_SPEED = 8
BALL_SPEED_X = 15
BALL_SPEED_Y = 5

class Paddle:
    def __init__(self, x, y, color):
        self.rect  = pygame.Rect(x, y, PADDLE_WIDTH, PADDLE_HEIGHT)
        self.color = color

    def move_up(self):
        self.rect.y = max(0, self.rect.y - PADDLE_SPEED)

    def move_down(self):
        self.rect.y = min(HEIGHT - PADDLE_HEIGHT, self.rect.y + PADDLE_SPEED)

    def ai_move(self, ball):
        if self.rect.centery < ball.rect.centery:
            self.move_down()
        elif self.rect.centery > ball.rect.centery:
            self.move_up()

    def draw(self):
        pygame.draw.rect(SCREEN, self.color, self.rect)

class Ball:
    def __init__(self):
        self.rect = pygame.Rect(
            WIDTH//2 - BALL_SIZE//2,
            HEIGHT//2 - BALL_SIZE//2,
            BALL_SIZE, BALL_SIZE
        )
        self.dx = random.choice([-BALL_SPEED_X, BALL_SPEED_X])
        self.dy = random.choice([-BALL_SPEED_Y, BALL_SPEED_Y])

    def move(self):
        self.rect.x += self.dx
        self.rect.y += self.dy
        if self.rect.top <= 0 or self.rect.bottom >= HEIGHT:
            self.dy *= -1

    def paddle_collision(self):
        self.dx *= -1

    def draw(self):
        pygame.draw.ellipse(SCREEN, COLOR_BALL, self.rect)
        pygame.draw.aaline(SCREEN, COLOR_TEXT, (WIDTH//2, 0), (WIDTH//2, HEIGHT))

    def reset(self):
        self.rect.center = (WIDTH//2, HEIGHT//2)
        self.dx *= -1
        self.dy = random.choice([-BALL_SPEED_Y, BALL_SPEED_Y])

class Button:
    def __init__(self, rect, text, action):
        self.rect   = pygame.Rect(rect)
        self.text   = text
        self.action = action

    def draw(self):
        pygame.draw.rect(SCREEN, COLOR_BUTTON, self.rect, border_radius=8)
        txt = FONT_BUTTON.render(self.text, True, COLOR_TEXT)
        SCREEN.blit(
            txt,
            (
                self.rect.x + (self.rect.w - txt.get_width())//2,
                self.rect.y + (self.rect.h - txt.get_height())//2
            )
        )

    def click(self, pos):
        if self.rect.collidepoint(pos):
            self.action()

def main():
    state      = STATE_MENU
    mode       = None
    name1 = name2 = ""
    input_text = ""

    # Menu buttons
    btn_pvp  = Button((WIDTH//2 -150, HEIGHT//2 -80, 300, 60),
                      "Player vs Player", lambda: select_mode("PVP"))
    btn_pvai = Button((WIDTH//2 -150, HEIGHT//2 +20, 300, 60),
                      "Player vs AI",     lambda: select_mode("PVAI"))
    buttons = [btn_pvp, btn_pvai]

    def select_mode(m):
        nonlocal state, mode
        mode  = m
        state = STATE_NAME1

    paddle1 = Paddle(20,               HEIGHT//2 - PADDLE_HEIGHT//2, COLOR_P1)
    paddle2 = Paddle(WIDTH-40,         HEIGHT//2 - PADDLE_HEIGHT//2, COLOR_P2)
    ball    = Ball()
    score1 = score2 = 0
    playing    = False

    while True:
        CLOCK.tick(FPS)
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                pygame.quit(); sys.exit()
            if event.type == pygame.MOUSEBUTTONDOWN and state == STATE_MENU:
                for b in buttons: b.click(event.pos)
            if event.type == pygame.KEYDOWN:
                if state in (STATE_NAME1, STATE_NAME2):
                    if event.key == pygame.K_RETURN:
                        if state == STATE_NAME1:
                            name1 = input_text or "Player 1"
                            input_text = ""
                            if mode == "PVP":
                                state = STATE_NAME2
                            else:
                                name2 = "AI"
                                state = STATE_PLAY
                        else:
                            name2 = input_text or "Player 2"
                            input_text = ""
                            state = STATE_PLAY
                        continue
                    elif event.key == pygame.K_BACKSPACE:
                        input_text = input_text[:-1]
                    else:
                        if len(input_text) < 12 and event.unicode.isprintable():
                            input_text += event.unicode
                elif state == STATE_PLAY:
                    if event.key == pygame.K_p:
                        playing = not playing
                    if event.key == pygame.K_r:
                        score1 = score2 = 0
                        ball.reset()
                        paddle1.rect.centery = paddle2.rect.centery = HEIGHT//2
                        playing = False

        SCREEN.fill(COLOR_BG)

        if state == STATE_MENU:
            title = FONT_TITLE.render("Pong Deluxe", True, COLOR_TEXT)
            SCREEN.blit(title, ((WIDTH - title.get_width())//2, 100))
            for b in buttons:
                b.draw()

        elif state in (STATE_NAME1, STATE_NAME2):
            prompt = "Enter name for " + ("Player 1:" if state == STATE_NAME1 else "Player 2:")
            txt1 = FONT_BUTTON.render(prompt, True, COLOR_TEXT)
            txt2 = FONT_BUTTON.render(input_text, True, COLOR_TEXT)
            SCREEN.blit(txt1, ((WIDTH-txt1.get_width())//2, HEIGHT//3))
            SCREEN.blit(txt2, ((WIDTH-txt2.get_width())//2, HEIGHT//2))

        elif state == STATE_PLAY:
            keys = pygame.key.get_pressed()
            if playing:
                # P1 controls
                if keys[pygame.K_w]: paddle1.move_up()
                if keys[pygame.K_s]: paddle1.move_down()
                # P2 or AI
                if mode == "PVP":
                    if keys[pygame.K_UP]:   paddle2.move_up()
                    if keys[pygame.K_DOWN]: paddle2.move_down()
                else:
                    paddle2.ai_move(ball)

                # Move and collide
                ball.move()
                if ball.rect.colliderect(paddle1.rect) or ball.rect.colliderect(paddle2.rect):
                    ball.paddle_collision()

                # Score check
                if ball.rect.left  <= 0:
                    score2 += 1; ball.reset(); playing = False
                if ball.rect.right >= WIDTH:
                    score1 += 1; ball.reset(); playing = False

            # Draw objects
            paddle1.draw()
            paddle2.draw()
            ball.draw()

            # Draw scores and serve prompt
            scr1 = FONT_SCORE.render(f"{name1 or 'P1'}: {score1}", True, COLOR_P1)
            scr2 = FONT_SCORE.render(f"{name2 or 'P2'}: {score2}", True, COLOR_P2)
            SCREEN.blit(scr1, (50, 20))
            SCREEN.blit(scr2, (WIDTH - scr2.get_width() - 50, 20))
            if not playing:
                msg = FONT_NAME.render("Press P to Serve, R to Reset", True, COLOR_TEXT)
                SCREEN.blit(msg, ((WIDTH - msg.get_width())//2, HEIGHT - 40))

        pygame.display.flip()

if __name__ == '__main__':
    main()
